#include "TableHeap/TableHeap.hpp"
#include "BufferPoolManager/BufferPoolManager.hpp"
#include <iostream>

int main() {
    BufferPoolManager bpm(3);
    std::string table_name = "test_table";

    TableHeap heap(&bpm, table_name);

    heap.insertRow("row 2 is also here.");
    heap.insertRow("row 3, filling up.");
    heap.insertRow("row 4, maybe new page?");

    
    auto all_rows = heap.scanAllRows();
    for (int i = 0; i < all_rows.size(); ++i) {
        std::cout << "Row " << i << ": " << all_rows[i] << "\n";
    }

    bpm.shutdown();
    return 0;
}
