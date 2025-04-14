// test.cpp
#include "TableHeap/TableHeap.hpp"
#include "BufferPoolManager/BufferPoolManager.hpp"
#include "CatalogManager/CatalogManager.hpp"
#include "RecordManager/RecordManager.hpp"

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

void createTableIfNotExists(CatalogManager& catalog, const std::string& table_name, int num_columns) {
    auto metadata = catalog.getTableMetadata(table_name);

    if (!metadata.has_value()) {

        TableMetadata tmd;
        tmd.table_name = table_name;
        tmd.table_id = CatalogManager::next_table_id++;
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

int main() {    
    db::Column id_col("id", db::TypeID::INTEGER, 0, false);       
    db::Column name_col("name", db::TypeID::VARCHAR, 50, false); 
    db::Column gpa_col("gpa", db::TypeID::FLOAT, 0, true);        
    
    std::vector<db::Column> columns = {id_col, name_col, gpa_col};
    db::Schema student_schema(columns);
    
    std::cout << "Column count: " << student_schema.GetColumnCount() << std::endl;
    std::cout << "Schema length: " << student_schema.GetLength() << " bytes" << std::endl;
    
    // Find a column by name
    int32_t name_idx = student_schema.GetColumnIndex("name");
    if (name_idx != -1) {
        std::cout << "Name column is at index: " << name_idx << std::endl;
    }
    
    BufferPoolManager bpm(10000);

    CatalogManager catalog;

    const std::string table_name = "Student";
    const int num_columns = 3;

    createTableIfNotExists(catalog, table_name, num_columns);

    catalog.addSchema(table_name, student_schema);
    
    
    // TableHeap heap(&bpm, table_name);

    // std::vector<std::string> sample_rows = {
    //     "Col1_data1!@#$Col2_data1!@#$Col3_data1",
    //     "Col1_data2!@#$Col2_data2!@#$Col3_data2",
    //     "Col1_data3!@#$Col2_data3!@#$Col3_data3",
    //     "Col1_data4!@#$Col2_data4!@#$Col3_data4"
    // };

    // for (const auto& row : sample_rows) {
    //     bool success = heap.insertRow(row);
    //     if (!success) {
    //         std::cerr << "⚠️ Failed to insert row: " << row << "\n";
    //     }
    // }
    
    // schema for teachers
    db::Column id_col_t("id", db::TypeID::INTEGER, 0, false);
    db::Column name_col_t("name", db::TypeID::VARCHAR, 50, false);
    db::Column subject_col("subject", db::TypeID::VARCHAR, 50, false);
    db::Column salary_col("salary", db::TypeID::FLOAT, 0, true);

    std::vector<db::Column> columns_t = {id_col_t, name_col_t, subject_col, salary_col};
    db::Schema teacher_schema(columns_t);
    std::cout << "Column count: " << teacher_schema.GetColumnCount() << std::endl;
    std::cout << "Schema length: " << teacher_schema.GetLength() << " bytes" << std::endl;
    int32_t subject_idx = teacher_schema.GetColumnIndex("subject");
    if (subject_idx != -1) {
        std::cout << "Subject column is at index: " << subject_idx << std::endl;
    }
    // Create a new table for teachers
    const std::string teacher_table_name = "Teacher";
    createTableIfNotExists(catalog, teacher_table_name, columns_t.size());
    catalog.addSchema(teacher_table_name, teacher_schema);
    TableHeap heap_t(&bpm, teacher_table_name);
    std::vector<std::string> sample_rows_t = {
        "Col1_data1!@#$Col2_data1!@#$Col3_data1!@#$Col4_data1",
        "Col1_data2!@#$Col2_data2!@#$Col3_data2!@#$Col4_data2",
        "Col1_data3!@#$Col2_data3!@#$Col3_data3!@#$Col4_data3",
        "Col1_data4!@#$Col2_data4!@#$Col3_data4!@#$Col4_data4"
    };
    for (const auto& row : sample_rows_t) {
        bool success = heap_t.insertRow(row);
        if (!success) {
            std::cerr << "⚠️ Failed to insert row: " << row << "\n";
        }
    }

    printTable(heap_t);
    bpm.shutdown();
    std::cout << "\n✅ BufferPoolManager shutdown completed.\n";


    return 0;
}