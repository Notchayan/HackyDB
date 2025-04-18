#include "CatalogManager.hpp"
#include <fstream>
#include "../include/json.hpp"
#include <iostream>

using json = nlohmann::json;

int CatalogManager::next_table_id = 0;
int CatalogManager::next_column_id = 0;
int CatalogManager::next_index_id = 0;

CatalogManager::CatalogManager() {
    loadCatalog();
}

void CatalogManager::loadCatalog() {
    loadnextids();
    loadTables();
    loadColumns();
    loadIndexes();
    loadSchemas();
}

void CatalogManager::saveCatalog() {
    savenextids();
    saveTables();
    saveColumns();
    saveIndexes();
    saveSchemas();

}

CatalogManager::~CatalogManager() {
    saveCatalog();
}

void CatalogManager::loadnextids() {
    std::ifstream in("catalog_ids.meta");
    if (!in.is_open()) {
        next_table_id = 0;
        next_column_id = 0;
        next_index_id = 0;
        return;
    }
    json j;
    in >> j;
    next_table_id = j.value("next_table_id", 0);
    next_column_id = j.value("next_column_id", 0);
    next_index_id = j.value("next_index_id", 0);
}

void CatalogManager::savenextids() {
    std::ofstream out("catalog_ids.meta");
    json j;
    j["next_table_id"] = next_table_id;
    j["next_column_id"] = next_column_id;
    j["next_index_id"] = next_index_id;
    out << j.dump(4);
}

bool CatalogManager::addTableMetadata(const TableMetadata& table) {
    if (table_map.count(table.table_name)) return false;
    table_map[table.table_name] = table;
    return true;
}

bool CatalogManager::addColumnMetadata(const ColumnMetadata& column) {
    column_map[column.table_name].push_back(column);
    return true;
}

bool CatalogManager::addIndexMetadata(const IndexMetadata& index) {
    if (index_map.count(index.index_name)) return false;
    index_map[index.index_name] = index;
    return true;
}

std::optional<TableMetadata> CatalogManager::getTableMetadata(const std::string& name) {
    if (!table_map.count(name)) return std::nullopt;
    return table_map[name];
}

std::vector<ColumnMetadata> CatalogManager::getColumnMetadata(const std::string& table_name) {
    if (!column_map.count(table_name)) return {};
    return column_map[table_name];
}

std::optional<IndexMetadata> CatalogManager::getIndexMetadata(const std::string& name) {
    if (!index_map.count(name)) return std::nullopt;
    return index_map[name];
}

bool CatalogManager::addSchema(const std::string &table_name, const db::Schema &schema) {
    schema_map[table_name] = schema;
    std::cout << "addSchema called for table: " << table_name << "\n";
    return true;
}

std::optional<db::Schema> CatalogManager::getSchema(const std::string &table_name) const {
    auto it = schema_map.find(table_name);
    if (it != schema_map.end()) {
        return it->second;
    }
    return std::nullopt;
}

db::Schema CatalogManager::constructSchemaFromColumns(const std::string &table_name) {
    auto col_meta = getColumnMetadata(table_name);
    std::vector<db::Column> cols;

    for (const auto &meta : col_meta) {
        db::TypeID type;
        if (meta.data_type == "INTEGER") type = db::TypeID::INTEGER;
        else if (meta.data_type == "BOOLEAN") type = db::TypeID::BOOLEAN;
        else if (meta.data_type == "FLOAT") type = db::TypeID::FLOAT;
        else if (meta.data_type == "VARCHAR") type = db::TypeID::VARCHAR;
        else type = db::TypeID::INVALID;

        cols.emplace_back(meta.column_name, type, meta.data_type == "VARCHAR" ? 255 : 0, meta.is_nullable);
    }

    return db::Schema(cols);
}


void CatalogManager::saveSchemas() {
    std::ofstream out("schemas.meta");
    for (const auto &[table_name, schema] : schema_map) {
        out << table_name << " " << schema.GetColumnCount() << "\n";
        for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
            const auto &col = schema.GetColumn(i);
            out << col.GetName() << " "
                << static_cast<int>(col.GetType()) << " "
                << col.GetLength() << " "
                << col.IsNullable() << "\n";
        }
    }
    std::cout << "saveSchemas called\n";
}

void CatalogManager::loadSchemas() {
    std::ifstream in("schemas.meta");
    std::string table_name;
    uint32_t column_count;

    while (in >> table_name >> column_count) {
        std::vector<db::Column> cols;
        for (uint32_t i = 0; i < column_count; ++i) {
            std::string name;
            int type;
            uint32_t length;
            bool nullable;
            in >> name >> type >> length >> nullable;
            cols.emplace_back(name, static_cast<db::TypeID>(type), length, nullable);
        }
        db::Schema schema(cols);
        schema_map[table_name] = schema;
    }
}


// ------------------ LOAD -------------------

void CatalogManager::loadTables() {
    std::ifstream in("catalog_tables.meta");
    if (!in.is_open()) return;

    json j;
    in >> j;

    for (const auto& obj : j) {
        TableMetadata t;
        t.table_name = obj["table_name"];
        t.table_id = obj["table_id"];
        t.row_size = obj["row_size"];
        t.num_columns = obj["num_columns"];
        table_map[t.table_name] = t;
    }
}

void CatalogManager::loadColumns() {
    std::ifstream in("catalog_columns.meta");
    if (!in.is_open()) return;

    json j;
    in >> j;

    for (const auto& obj : j) {
        ColumnMetadata c;
        c.table_name = obj["table_name"];
        c.column_name = obj["column_name"];
        c.data_type = obj["data_type"];
        c.is_nullable = obj["is_nullable"];
        c.is_primary_key = obj["is_primary_key"];
        c.default_value = obj["default_value"];
        column_map[c.table_name].push_back(c);
    }
}

void CatalogManager::loadIndexes() {
    std::ifstream in("catalog_indexes.meta");
    if (!in.is_open()) return;

    json j;
    in >> j;

    for (const auto& obj : j) {
        IndexMetadata idx;
        idx.index_name = obj["index_name"];
        idx.table_name = obj["table_name"];
        idx.column_names = obj["column_names"].get<std::vector<std::string>>();
        idx.is_unique = obj["is_unique"];
        idx.index_type = obj["index_type"];
        idx.index_file = obj["index_file"];
        index_map[idx.index_name] = idx;
    }
}

// ------------------ SAVE -------------------

void CatalogManager::saveTables() {
    std::ofstream out("catalog_tables.meta");
    json j = json::array();

    for (const auto& [_, t] : table_map) {
        j.push_back({
            {"table_name", t.table_name},
            {"table_id", t.table_id},
            {"row_size", t.row_size},
            {"num_columns", t.num_columns}
        });
    }

    out << j.dump(4);
}

void CatalogManager::saveColumns() {
    std::ofstream out("catalog_columns.meta");
    json j = json::array();

    for (const auto& [_, columns] : column_map) {
        for (const auto& c : columns) {
            j.push_back({
                {"table_name", c.table_name},
                {"column_name", c.column_name},
                {"data_type", c.data_type},
                {"is_nullable", c.is_nullable},
                {"is_primary_key", c.is_primary_key},
                {"default_value", c.default_value}
            });
        }
    }

    out << j.dump(4);
}

void CatalogManager::saveIndexes() {
    std::ofstream out("catalog_indexes.meta");
    json j = json::array();

    for (const auto& [_, idx] : index_map) {
        j.push_back({
            {"index_name", idx.index_name},
            {"table_name", idx.table_name},
            {"column_names", idx.column_names},
            {"is_unique", idx.is_unique},
            {"index_type", idx.index_type},
            {"index_file", idx.index_file}
        });
    }

    out << j.dump(4);
}

std::unordered_map<std::string, TableMetadata> CatalogManager::getAllTables() const {
    return table_map;
}
