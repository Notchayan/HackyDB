#include "TableHeap/TableHeap.hpp"
#include "BufferPoolManager/BufferPoolManager.hpp"
#include <iostream>

int main() {
    BufferPoolManager bpm(10000);
    std::string table_name = "test_table";
    std::string table_name2 = "test_table_2";

    TableHeap heap(&bpm, table_name);

    heap.insertRow("row 2 is also here.");
    heap.insertRow("row 3, filling up.");
    heap.insertRow("row 4, maybe new page?");


    auto all_rows = heap.scanAllRows();
    for (int i = 0; i < all_rows.size(); ++i) {
        std::cout << "Row " << i << ": " << all_rows[i] << "\n";
    }

    TableHeap heap2(&bpm, table_name2);
    heap2.insertRow("This is the fucking row 0 of the second table.");
    heap2.insertRow("This is the fucking row 1 of the second table.");
    heap2.insertRow("This is the fucking row 2 of the second table.");
    heap2.insertRow("This is the fucking row 3 of the second table.");

    auto all_rows2 = heap2.scanAllRows();
    for(int i = 0; i < all_rows2.size(); ++i) {
        std::cout << "Row " << i << ": " << all_rows2[i] << "\n";
    }

    bpm.shutdown();
    return 0;
}
