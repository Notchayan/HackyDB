// TableHeap.hpp

#ifndef TABLE_HEAP_HPP
#define TABLE_HEAP_HPP

#include <string>
#include <vector>
#include <mutex>
#include "../BufferPoolManager/BufferPoolManager.hpp"

const int ROW_SIZE = 100;

class TableHeap {
    BufferPoolManager* bpm;
    std::string table_name;
    int next_logical_page;

public:
    TableHeap(BufferPoolManager* bpm, const std::string& table_name);

    // Insert a row, returns true if inserted
    bool insertRow(const std::string& row);

    // Returns all rows stored in the table
    std::vector<std::string> scanAllRows();
};

#endif // TABLE_HEAP_HPP
