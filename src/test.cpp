#include "BufferPoolManager/BufferPoolManager.hpp"
#include <iostream>

int main() {
    BufferPoolManager bpm(3); // Create a buffer pool with 3 frames
    std::string table_name = "test_table";

    // Allocate and write to a page
    allocatePage(table_name, 0);
    Page* page = bpm.fetchPage(table_name, 0);
    if (page) {
        std::string content = "Hello, BufferPool!";
        strncpy(page->data, content.c_str(), PAGE_SIZE - 1);
        page->data[PAGE_SIZE - 1] = '\0'; // Ensure null-termination
        bpm.unpinPage(page->page_id, true);
    }

    // Read back the page
    page = bpm.fetchPage(table_name, 0);
    if (page) {
        std::cout << "Page content: " << page->data << std::endl;
        bpm.unpinPage(page->page_id, false);
    }

    bpm.shutdown();
    return 0;
}