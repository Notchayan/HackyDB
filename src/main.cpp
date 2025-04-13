#include "TableHeap/TableHeap.hpp"
#include "BufferPoolManager/BufferPoolManager.hpp"
#include "CatalogManager/CatalogManager.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <optional>

const std::string DELIMITER = "!@#$";

// Parses a delimited row string into individual column values
std::vector<std::string> parseRow(const std::string& row) {
    std::vector<std::string> columns;
    size_t start = 0;
    size_t end = row.find(DELIMITER);

    while (end != std::string::npos) {
        columns.emplace_back(row.substr(start, end - start));
        start = end + DELIMITER.length();
        end = row.find(DELIMITER, start);
    }
    columns.emplace_back(row.substr(start));
    return columns;
}

// Helper to create metadata if table doesn't already exist
void createTableIfNotExists(CatalogManager& catalog, const std::string& table_name, int num_columns) {
    auto metadata = catalog.getTableMetadata(table_name);

    if (!metadata.has_value()) {
        // Compute a new table ID safely
        int next_id = 1;
        auto all_tables = catalog.getAllTables();  // returns map<string, TableMetadata>
        for (const auto& [name, meta] : all_tables) {
            next_id = std::max(next_id, meta.table_id + 1);
        }

        TableMetadata tmd;
        tmd.table_name = table_name;
        tmd.table_id = next_id;
        tmd.row_size = num_columns;
        tmd.num_columns = num_columns;

        catalog.addTableMetadata(tmd);
        catalog.saveCatalog();

        std::cout << "✅ Created new table metadata: '" << table_name << "' with ID " << tmd.table_id << "\n";
    } else {
        std::cout << "📁 Loaded existing table metadata for '" << table_name << "' (ID: " 
                  << metadata->table_id << ")\n";
    }
}


// Displays table content beautifully
void printTable(TableHeap& heap) {
    auto rows = heap.scanAllRows();
    std::cout << "\n📊 Table Contents:\n";
    int row_id = 1;
    for (const auto& row : rows) {
        auto columns = parseRow(row);
        std::cout << "Row " << row_id++ << ": ";
        for (const auto& col : columns) {
            std::cout << "[" << col << "] ";
        }
        std::cout << "\n";
    }
}

int main() {
    try {
        BufferPoolManager bpm(10000);
        CatalogManager catalog;

        const std::string table_name = "test_table";
        const int num_columns = 3;

        createTableIfNotExists(catalog, table_name, num_columns);

        TableHeap heap(&bpm, table_name);

        // Insert some rows
        std::vector<std::string> sample_rows = {
            "Col1_data1!@#$Col2_data1!@#$Col3_data1",
            "Col1_data2!@#$Col2_data2!@#$Col3_data2",
            "Col1_data3!@#$Col2_data3!@#$Col3_data3",
            "Col1_data4!@#$Col2_data4!@#$Col3_data4"
        };

        for (const auto& row : sample_rows) {
            bool success = heap.insertRow(row);
            if (!success) {
                std::cerr << "⚠️ Failed to insert row: " << row << "\n";
            }
        }

        printTable(heap);
        bpm.shutdown();
        std::cout << "\n✅ BufferPoolManager shutdown completed.\n";

    } catch (const std::exception& ex) {
        std::cerr << "❌ Exception occurred: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
