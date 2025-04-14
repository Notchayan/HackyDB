#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_map>

namespace db {

/**
 * TypeID - The possible types of values in the database system
 */
enum class TypeID {
    INVALID,
    BOOLEAN,
    INTEGER,
    FLOAT,
    VARCHAR,
    NULL_TYPE
};

/**
 * Column - Information about a single column in a schema
 */
class Column {
public:
    Column(std::string name, TypeID type, uint32_t length = 0, bool nullable = false)
        : name_(std::move(name)), type_(type), length_(length), nullable_(nullable) {}

    const std::string &GetName() const { return name_; }
    TypeID GetType() const { return type_; }
    uint32_t GetLength() const { return length_; }
    bool IsNullable() const { return nullable_; }
    
    // Size of the type in bytes
    uint32_t GetSize() const;

    // Is the type fixed length or variable length?
    bool IsFixedLength() const;

private:
    std::string name_;
    TypeID type_;
    uint32_t length_; // For VARCHAR, this is the max length
    bool nullable_;
};

/**
 * Schema - Metadata about the structure of a tuple
 */
class Schema {
public:
    Schema() = default;
    Schema(std::vector<Column> columns);
    
    // Add a column to the schema
    void AddColumn(Column col);
    
    // Get the column at the given index
    const Column &GetColumn(uint32_t idx) const;
    
    // Get index of column by name (-1 if not found)
    int32_t GetColumnIndex(const std::string &name) const;
    
    // Get the number of columns
    uint32_t GetColumnCount() const;
    
    // Get the size of a tuple with this schema
    uint32_t GetLength() const;
    
    // Get schema with subset of columns
    Schema SelectSubset(const std::vector<uint32_t> &column_indices) const;

private:
    std::vector<Column> columns_;
    std::unordered_map<std::string, uint32_t> name_to_index_;
    uint32_t tuple_length_;
};

/**
 * Value - A value of a specific type
 */
class Value {
public:
    // Constructors
    Value() : type_id_(TypeID::INVALID) {}
    
    // Create a null value of the specified type
    static Value CreateNull(TypeID type_id);
    
    // Factory methods for creating values
    static Value CreateBoolean(bool val);
    static Value CreateInteger(int32_t val);
    static Value CreateFloat(float val);
    static Value CreateVarchar(const std::string &val);
    
    // Access methods
    bool IsNull() const { return is_null_; }
    TypeID GetTypeId() const { return type_id_; }
    
    // Get value methods
    bool GetAsBoolean() const;
    int32_t GetAsInteger() const;
    float GetAsFloat() const;
    const std::string &GetAsVarchar() const;
    
    // For serialization
    uint32_t SerializeSize() const;
    void SerializeTo(char *storage) const;
    static Value DeserializeFrom(const char *storage, TypeID type_id, bool is_null = false);

private:
    TypeID type_id_;
    bool is_null_ = false;
    
    // Union to store the actual value
    union {
        bool boolean_val_;
        int32_t integer_val_;
        float float_val_;
    };
    
    // For VARCHAR (can't be in union with pointer types)
    std::string varchar_val_;
};

/**
 * RID - Record ID for locating a tuple
 * Format: (page_id, slot_num)
 */
class RID {
public:
    RID() : page_id_(0), slot_num_(0) {}
    RID(uint32_t page_id, uint32_t slot_num) : page_id_(page_id), slot_num_(slot_num) {}
    
    uint32_t GetPageId() const { return page_id_; }
    uint32_t GetSlotNum() const { return slot_num_; }
    
    bool operator==(const RID &other) const {
        return page_id_ == other.page_id_ && slot_num_ == other.slot_num_;
    }
    
    bool operator!=(const RID &other) const {
        return !(*this == other);
    }
    
    // For serialization
    std::string ToString() const;
    
private:
    uint32_t page_id_;
    uint32_t slot_num_;
};

/**
 * Tuple - A single record in the database
 */
class Tuple {
public:
    // Create an empty tuple
    Tuple() = default;
    
    // Create a tuple from values
    Tuple(std::vector<Value> values, const Schema *schema);
    
    // Get the value at the given column index
    Value GetValue(uint32_t column_idx) const;
    
    // Get the RID of this tuple
    RID GetRid() const { return rid_; }
    
    // Set the RID of this tuple
    void SetRid(const RID &rid) { rid_ = rid; }
    
    // Get size of the tuple in bytes when serialized
    uint32_t GetSize() const;
    
    // Serialize tuple data to raw memory
    bool SerializeTo(char *storage) const;
    
    // Deserialize tuple data from raw memory
    bool DeserializeFrom(const char *storage, const Schema *schema);
    
    // Project tuple onto a subset of columns
    Tuple ProjectedTuple(const Schema &schema, const Schema &target_schema, 
                       const std::vector<uint32_t> &column_indices) const;
    
    // For debugging
    std::string ToString(const Schema *schema) const;

private:
    // The actual data values
    std::vector<Value> values_;
    
    // Record ID of this tuple
    RID rid_;
    
    // Reference to the schema (owned elsewhere)
    const Schema *schema_ = nullptr;
};

/**
 * TupleMeta - Metadata about a tuple (used for MVCC)
 * Can be extended for transaction management
 */
struct TupleMeta {
    bool is_deleted = false;
};

} // namespace db