#ifndef CATALOG_MANAGER_HPP
#define CATALOG_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

struct TableMetadata {
    std::string table_name;
    int table_id;
    int row_size;
    int num_columns;
};

struct ColumnMetadata {
    std::string table_name;
    std::string column_name;
    std::string data_type;
    bool is_nullable;
    bool is_primary_key;
    std::string default_value;
};

struct IndexMetadata {
    std::string index_name;
    std::string table_name;
    std::vector<std::string> column_names;
    bool is_unique;
    std::string index_type;
    std::string index_file;
};

class CatalogManager {
public:
    CatalogManager();

    void loadCatalog();
    void saveCatalog();

    bool addTableMetadata(const TableMetadata& table);
    bool addColumnMetadata(const ColumnMetadata& column);
    bool addIndexMetadata(const IndexMetadata& index);

    std::optional<TableMetadata> getTableMetadata(const std::string& table_name);
    std::vector<ColumnMetadata> getColumnMetadata(const std::string& table_name);
    std::optional<IndexMetadata> getIndexMetadata(const std::string& index_name);

private:
    std::unordered_map<std::string, TableMetadata> table_map;
    std::unordered_map<std::string, std::vector<ColumnMetadata>> column_map;
    std::unordered_map<std::string, IndexMetadata> index_map;

    void loadTables();
    void saveTables();

    void loadColumns();
    void saveColumns();

    void loadIndexes();
    void saveIndexes();
};

#endif // CATALOG_MANAGER_HPP
