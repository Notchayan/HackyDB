#include "CatalogManager.hpp"
#include <fstream>
#include "../include/json.hpp"

using json = nlohmann::json;

CatalogManager::CatalogManager() {
    loadCatalog();
}

void CatalogManager::loadCatalog() {
    loadTables();
    loadColumns();
    loadIndexes();
}

void CatalogManager::saveCatalog() {
    saveTables();
    saveColumns();
    saveIndexes();
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
