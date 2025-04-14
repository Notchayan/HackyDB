#include "RecordManager.hpp"
#include <cstring>
#include <cassert>
#include <sstream>
#include <iomanip>

namespace db {

// ===== Column Implementation =====

uint32_t Column::GetSize() const {
    switch (type_) {
        case TypeID::BOOLEAN:
            return sizeof(bool);
        case TypeID::INTEGER:
            return sizeof(int32_t);
        case TypeID::FLOAT:
            return sizeof(float);
        case TypeID::VARCHAR:
            return length_ + sizeof(uint32_t); // Size includes length prefix
        case TypeID::NULL_TYPE:
        case TypeID::INVALID:
        default:
            return 0;
    }
}

bool Column::IsFixedLength() const {
    return type_ != TypeID::VARCHAR;
}

// ===== Schema Implementation =====

Schema::Schema(std::vector<Column> columns) : columns_(std::move(columns)) {
    tuple_length_ = 0;
    
    // Build the name to index map and calculate tuple length
    for (uint32_t i = 0; i < columns_.size(); i++) {
        name_to_index_[columns_[i].GetName()] = i;
        tuple_length_ += columns_[i].GetSize();
        
        // Add space for null bitmap, 1 bit per column
        // We'll round up to the nearest byte
        tuple_length_ = (columns_.size() + 7) / 8;
    }
}

void Schema::AddColumn(Column col) {
    name_to_index_[col.GetName()] = columns_.size();
    columns_.push_back(std::move(col));
    
    // Recalculate tuple length
    tuple_length_ = 0;
    for (const auto &column : columns_) {
        tuple_length_ += column.GetSize();
    }
    
    // Add space for null bitmap (1 bit per column, rounded up to byte)
    tuple_length_ += (columns_.size() + 7) / 8;
}

const Column &Schema::GetColumn(uint32_t idx) const {
    assert(idx < columns_.size());
    return columns_[idx];
}

int32_t Schema::GetColumnIndex(const std::string &name) const {
    auto it = name_to_index_.find(name);
    if (it == name_to_index_.end()) {
        return -1;
    }
    return static_cast<int32_t>(it->second);
}

uint32_t Schema::GetColumnCount() const {
    return columns_.size();
}

uint32_t Schema::GetLength() const {
    return tuple_length_;
}

Schema Schema::SelectSubset(const std::vector<uint32_t> &column_indices) const {
    std::vector<Column> subset_columns;
    subset_columns.reserve(column_indices.size());
    
    for (auto idx : column_indices) {
        if (idx < columns_.size()) {
            subset_columns.push_back(columns_[idx]);
        }
    }
    
    return Schema(subset_columns);
}

// ===== Value Implementation =====

Value Value::CreateNull(TypeID type_id) {
    Value val;
    val.type_id_ = type_id;
    val.is_null_ = true;
    return val;
}

Value Value::CreateBoolean(bool val) {
    Value value;
    value.type_id_ = TypeID::BOOLEAN;
    value.boolean_val_ = val;
    return value;
}

Value Value::CreateInteger(int32_t val) {
    Value value;
    value.type_id_ = TypeID::INTEGER;
    value.integer_val_ = val;
    return value;
}

Value Value::CreateFloat(float val) {
    Value value;
    value.type_id_ = TypeID::FLOAT;
    value.float_val_ = val;
    return value;
}

Value Value::CreateVarchar(const std::string &val) {
    Value value;
    value.type_id_ = TypeID::VARCHAR;
    value.varchar_val_ = val;
    return value;
}

bool Value::GetAsBoolean() const {
    assert(type_id_ == TypeID::BOOLEAN && !is_null_);
    return boolean_val_;
}

int32_t Value::GetAsInteger() const {
    assert(type_id_ == TypeID::INTEGER && !is_null_);
    return integer_val_;
}

float Value::GetAsFloat() const {
    assert(type_id_ == TypeID::FLOAT && !is_null_);
    return float_val_;
}

const std::string &Value::GetAsVarchar() const {
    assert(type_id_ == TypeID::VARCHAR && !is_null_);
    return varchar_val_;
}

uint32_t Value::SerializeSize() const {
    if (is_null_) {
        return 0;  // Null values are tracked in the null bitmap, not in the data
    }
    
    switch (type_id_) {
        case TypeID::BOOLEAN:
            return sizeof(bool);
        case TypeID::INTEGER:
            return sizeof(int32_t);
        case TypeID::FLOAT:
            return sizeof(float);
        case TypeID::VARCHAR:
            return sizeof(uint32_t) + varchar_val_.size();  // Length prefix + data
        default:
            return 0;
    }
}

void Value::SerializeTo(char *storage) const {
    if (is_null_) {
        return;  // Null values don't need to be serialized
    }
    
    switch (type_id_) {
        case TypeID::BOOLEAN:
            memcpy(storage, &boolean_val_, sizeof(bool));
            break;
        case TypeID::INTEGER:
            memcpy(storage, &integer_val_, sizeof(int32_t));
            break;
        case TypeID::FLOAT:
            memcpy(storage, &float_val_, sizeof(float));
            break;
        case TypeID::VARCHAR: {
            // Write length prefix
            uint32_t length = varchar_val_.size();
            memcpy(storage, &length, sizeof(uint32_t));
            // Write actual string data
            memcpy(storage + sizeof(uint32_t), varchar_val_.data(), length);
            break;
        }
        default:
            break;
    }
}

Value Value::DeserializeFrom(const char *storage, TypeID type_id, bool is_null) {
    Value value;
    value.type_id_ = type_id;
    value.is_null_ = is_null;
    
    if (is_null) {
        return value;
    }
    
    switch (type_id) {
        case TypeID::BOOLEAN:
            memcpy(&value.boolean_val_, storage, sizeof(bool));
            break;
        case TypeID::INTEGER:
            memcpy(&value.integer_val_, storage, sizeof(int32_t));
            break;
        case TypeID::FLOAT:
            memcpy(&value.float_val_, storage, sizeof(float));
            break;
        case TypeID::VARCHAR: {
            // Read length prefix
            uint32_t length;
            memcpy(&length, storage, sizeof(uint32_t));
            // Read actual string data
            value.varchar_val_.assign(storage + sizeof(uint32_t), length);
            break;
        }
        default:
            break;
    }
    
    return value;
}

// ===== RID Implementation =====

std::string RID::ToString() const {
    std::stringstream ss;
    ss << "(" << page_id_ << ", " << slot_num_ << ")";
    return ss.str();
}

// ===== Tuple Implementation =====

Tuple::Tuple(std::vector<Value> values, const Schema *schema)
    : values_(std::move(values)), schema_(schema) {
    assert(schema_ != nullptr);
    assert(values_.size() == schema_->GetColumnCount());
}

Value Tuple::GetValue(uint32_t column_idx) const {
    assert(column_idx < values_.size());
    return values_[column_idx];
}

uint32_t Tuple::GetSize() const {
    if (schema_ == nullptr) {
        return 0;
    }
    
    // Calculate size: null bitmap + actual data
    uint32_t size = (schema_->GetColumnCount() + 7) / 8; // Null bitmap (rounded up to byte)
    
    for (uint32_t i = 0; i < values_.size(); i++) {
        if (!values_[i].IsNull()) {
            size += values_[i].SerializeSize();
        }
    }
    
    return size;
}

bool Tuple::SerializeTo(char *storage) const {
    if (schema_ == nullptr || values_.empty()) {
        return false;
    }
    
    uint32_t column_count = schema_->GetColumnCount();
    
    // First, write the null bitmap
    uint32_t null_bitmap_size = (column_count + 7) / 8;
    char null_bitmap[null_bitmap_size];
    memset(null_bitmap, 0, null_bitmap_size);
    
    for (uint32_t i = 0; i < column_count; i++) {
        if (values_[i].IsNull()) {
            // Set the bit in the null bitmap
            null_bitmap[i / 8] |= (1 << (i % 8));
        }
    }
    
    memcpy(storage, null_bitmap, null_bitmap_size);
    
    // Then, write the actual data
    char *data_ptr = storage + null_bitmap_size;
    for (uint32_t i = 0; i < column_count; i++) {
        if (!values_[i].IsNull()) {
            values_[i].SerializeTo(data_ptr);
            data_ptr += values_[i].SerializeSize();
        }
    }
    
    return true;
}

bool Tuple::DeserializeFrom(const char *storage, const Schema *schema) {
    if (schema == nullptr) {
        return false;
    }
    
    schema_ = schema;
    uint32_t column_count = schema_->GetColumnCount();
    values_.resize(column_count);
    
    // First, read the null bitmap
    uint32_t null_bitmap_size = (column_count + 7) / 8;
    const char *null_bitmap = storage;
    
    // Then, read the actual data
    const char *data_ptr = storage + null_bitmap_size;
    
    for (uint32_t i = 0; i < column_count; i++) {
        bool is_null = (null_bitmap[i / 8] & (1 << (i % 8))) != 0;
        TypeID type_id = schema_->GetColumn(i).GetType();
        
        if (is_null) {
            values_[i] = Value::CreateNull(type_id);
        } else {
            values_[i] = Value::DeserializeFrom(data_ptr, type_id);
            data_ptr += values_[i].SerializeSize();
        }
    }
    
    return true;
}

Tuple Tuple::ProjectedTuple(const Schema &schema, const Schema &target_schema, 
                           const std::vector<uint32_t> &column_indices) const {
    std::vector<Value> projected_values;
    projected_values.reserve(column_indices.size());
    
    for (auto idx : column_indices) {
        if (idx < values_.size()) {
            projected_values.push_back(values_[idx]);
        }
    }
    
    return Tuple(projected_values, &target_schema);
}

std::string Tuple::ToString(const Schema *schema) const {
    if (schema == nullptr || values_.empty()) {
        return "()";
    }
    
    std::stringstream ss;
    ss << "(";
    
    for (uint32_t i = 0; i < values_.size(); i++) {
        if (i > 0) {
            ss << ", ";
        }
        
        if (values_[i].IsNull()) {
            ss << "NULL";
            continue;
        }
        
        switch (values_[i].GetTypeId()) {
            case TypeID::BOOLEAN:
                ss << (values_[i].GetAsBoolean() ? "true" : "false");
                break;
            case TypeID::INTEGER:
                ss << values_[i].GetAsInteger();
                break;
            case TypeID::FLOAT:
                ss << std::fixed << std::setprecision(2) << values_[i].GetAsFloat();
                break;
            case TypeID::VARCHAR:
                ss << "'" << values_[i].GetAsVarchar() << "'";
                break;
            default:
                ss << "?";
                break;
        }
    }
    
    ss << ")";
    return ss.str();
}

} // namespace db