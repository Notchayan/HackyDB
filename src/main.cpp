#include "TableHeap/TableHeap.hpp"
#include "BufferPoolManager/BufferPoolManager.hpp"
#include <iostream>
#include <sstream>
#include <vector>

std::vector<std::string> parseRow(const std::string& row) {
    std::vector<std::string> columns;
    std::string delimiter = "!@#$";
    size_t start = 0;
    size_t end = row.find(delimiter);

    while (end != std::string::npos) {
        columns.push_back(row.substr(start, end - start));
        start = end + delimiter.length();
        end = row.find(delimiter, start);
    }
    columns.push_back(row.substr(start));
    return columns;
}

int main() {
    BufferPoolManager bpm(10000);
    std::string table_name = "test_table";

    TableHeap heap(&bpm, table_name);

    heap.insertRow("Col1_data1!@#$Col2_data1!@#$Col3_data1");
    heap.insertRow("Col1_data2!@#$Col2_data2!@#$Col3_data2");
    heap.insertRow("Col1_data3!@#$Col2_data3!@#$Col3_data3");
    heap.insertRow("Col1_data4!@#$Col2_data4!@#$Col3_data4");

    auto all_rows = heap.scanAllRows();
    for (const auto& row : all_rows) {
        auto columns = parseRow(row);
        for (const auto& col : columns) {
            std::cout << col << " ";
        }
        std::cout << std::endl;
    }

    bpm.shutdown();
    return 0;
}
