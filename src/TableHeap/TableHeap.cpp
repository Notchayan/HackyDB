#include "TableHeap.hpp"
#include <cstring>

TableHeap::TableHeap(BufferPoolManager* bpm, const std::string& table_name)
    : bpm(bpm), table_name(table_name), next_logical_page(0) {
    // std::lock_guard<std::mutex> lock(page_directory_mutex);
    if (page_directory.find(table_name) == page_directory.end()) {
        allocatePage(table_name, next_logical_page++);
    } else {
        next_logical_page = page_directory[table_name].size();
    }
}

bool TableHeap::insertRow(const std::string& row) {
    if (row.size() > ROW_SIZE) return false;

    for (int i = 0; i < next_logical_page; ++i) {
        Page* page = bpm->fetchPage(table_name, i);
        if (!page) return false;

        int* num_rows = reinterpret_cast<int*>(page->data);
        char* row_data_start = page->data + sizeof(int);

        if (*num_rows * ROW_SIZE + sizeof(int) + ROW_SIZE <= PAGE_SIZE) {
            std::memcpy(row_data_start + (*num_rows) * ROW_SIZE, row.c_str(), row.size());
            (*num_rows)++;
            bpm->unpinPage(page->page_id, true);
            return true;
        }

        bpm->unpinPage(page->page_id, false);
    }

    allocatePage(table_name, next_logical_page);
    Page* page = bpm->fetchPage(table_name, next_logical_page);
    if (!page) return false;

    int* num_rows = reinterpret_cast<int*>(page->data);
    *num_rows = 1;
    std::memcpy(page->data + sizeof(int), row.c_str(), row.size());

    bpm->unpinPage(page->page_id, true);
    next_logical_page++;
    return true;
}

std::vector<std::string> TableHeap::scanAllRows() {
    std::vector<std::string> rows;

    for (int i = 0; i < next_logical_page; ++i) {
        Page* page = bpm->fetchPage(table_name, i);
        if (!page) continue;

        int* num_rows = reinterpret_cast<int*>(page->data);
        char* row_data_start = page->data + sizeof(int);

        for (int j = 0; j < *num_rows; ++j) {
            std::string row(row_data_start + j * ROW_SIZE, ROW_SIZE);
            rows.push_back(row);
        }

        bpm->unpinPage(page->page_id, false);
    }

    return rows;
}
