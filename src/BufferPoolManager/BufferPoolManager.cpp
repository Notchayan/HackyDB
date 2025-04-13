#include "BufferPoolManager.hpp"
#include <cstring>
#include <iostream>

// Global variables
int next_page_id = 0;
int disk_fd = -1;
int next_physical_page_id = 0;
std::unordered_map<std::string, std::unordered_map<int, int>> page_directory;

std::mutex page_directory_mutex;
std::mutex physical_page_mutex;
std::mutex disk_io_mutex;

// Global function definitions
void openDiskFile() {
    disk_fd = open("dbfile", O_RDWR | O_CREAT, 0644);
    if (disk_fd < 0) {
        std::cerr << "Failed to open disk file: " << strerror(errno) << "\n";
        exit(1);
    }
}

void closeDiskFile() {
    if (disk_fd >= 0) {
        close(disk_fd);
        disk_fd = -1;
    }
}

void readPageFromDisk(int page_id, char* data) {
    off_t offset = page_id * PAGE_SIZE;
    if (lseek(disk_fd, offset, SEEK_SET) == -1) {
        std::cerr << "lseek failed in read: " << strerror(errno) << "\n";
        return;
    }

    ssize_t bytes = read(disk_fd, data, PAGE_SIZE);
    if (bytes < 0) {
        std::cerr << "read failed: " << strerror(errno) << "\n";
    } else if (bytes < PAGE_SIZE) {
        memset(data + bytes, 0, PAGE_SIZE - bytes); // Pad with zeros
    }
}

void writePageToDisk(int page_id, char* data) {
    off_t offset = page_id * PAGE_SIZE;
    if (lseek(disk_fd, offset, SEEK_SET) == -1) {
        std::cerr << "lseek failed in write: " << strerror(errno) << "\n";
        return;
    }

    ssize_t bytes = write(disk_fd, data, PAGE_SIZE);
    if (bytes < 0) {
        std::cerr << "write failed: " << strerror(errno) << "\n";
    }
}

void mapPage(const std::string& table_name, int logical_page_number, int physical_page_id) {
    std::lock_guard<std::mutex> lock(page_directory_mutex);
    page_directory[table_name][logical_page_number] = physical_page_id;
}

int allocatePage(const std::string& table_name, int logical_page_number) {
    std::lock_guard<std::mutex> pg_lock(physical_page_mutex);
    std::lock_guard<std::mutex> dir_lock(page_directory_mutex);
    int physical_page_id = next_physical_page_id++;
    page_directory[table_name][logical_page_number] = physical_page_id;

    char empty_data[PAGE_SIZE];
    memset(empty_data, 0, PAGE_SIZE);

    writePageToDisk(physical_page_id, empty_data);

    return physical_page_id;
}

// Page constructor
Page::Page() {
    memset(data, 0, PAGE_SIZE);
}

// LRUReplacer method definitions
void LRUReplacer::insert(int page_id) {
    std::lock_guard<std::mutex> lock(replacer_mutex);
    if (map.count(page_id)) {
        lru_list.erase(map[page_id]);
    }
    lru_list.push_front(page_id);
    map[page_id] = lru_list.begin();
}

void LRUReplacer::erase(int page_id) {
    std::lock_guard<std::mutex> lock(replacer_mutex);
    if (map.count(page_id)) {
        lru_list.erase(map[page_id]);
        map.erase(page_id);
    }
}

bool LRUReplacer::victim(int& page_id) {
    std::lock_guard<std::mutex> lock(replacer_mutex);
    if (lru_list.empty()) return false;
    page_id = lru_list.back();
    lru_list.pop_back();
    map.erase(page_id);
    return true;
}

// BufferPoolManager method definitions
BufferPoolManager::BufferPoolManager(int size) : pool_size(size) {
    openDiskFile();
    for (int i = 0; i < pool_size; ++i) {
        pages.push_back(new Page());
    }
}

BufferPoolManager::~BufferPoolManager() {
    for (auto page : pages) {
        flushPage(page->page_id);
        delete page;
    }
    closeDiskFile();
}

Page* BufferPoolManager::fetchPage(int page_id) {
    std::lock_guard<std::mutex> lock(fetch_mutex);
    {
        std::lock_guard<std::mutex> lock(page_table_mutex);
        if (page_table.count(page_id)) {
            Page* page = page_table[page_id];
            page->pin_count++;
            replacer.erase(page_id);
            return page;
        }
    }

    Page* frame = nullptr;
    {
        std::lock_guard<std::mutex> lock(pages_mutex);
        for (auto p : pages) {
            if (p->page_id == -1) {
                frame = p;
                break;
            }
        }
    }

    if (!frame) {
        int victim_id;
        if (!replacer.victim(victim_id)) {
            return nullptr;
        }
        {
            std::lock_guard<std::mutex> lock(page_table_mutex);
            frame = page_table[victim_id];
            if (frame->is_dirty) {
                flushPage(victim_id);
            }
            page_table.erase(victim_id);
        }
    }

    {
        std::lock_guard<std::mutex> disk_lock(disk_io_mutex);
        readPageFromDisk(page_id, frame->data);
    }

    {
        std::lock_guard<std::mutex> lock(page_table_mutex);
        frame->page_id = page_id;
        frame->pin_count = 1;
        frame->is_dirty = false;
        page_table[page_id] = frame;
    }
    return frame;
}

Page* BufferPoolManager::fetchPage(const std::string& table_name, int logical_page_number) {
    int physical_page_id;
    {
        std::lock_guard<std::mutex> lock(page_directory_mutex);
        if (!page_directory.count(table_name) || !page_directory[table_name].count(logical_page_number)) {
            std::cerr << "Page not found in page_directory for table " << table_name
                      << ", logical page " << logical_page_number << "\n";
            return nullptr;
        }
        physical_page_id = page_directory[table_name][logical_page_number];
    }
    return fetchPage(physical_page_id);
}

void BufferPoolManager::unpinPage(int page_id, bool is_dirty) {
    std::lock_guard<std::mutex> lock(page_table_mutex);
    if (!page_table.count(page_id)) return;
    Page* page = page_table[page_id];
    if (page->pin_count > 0) page->pin_count--;
    page->is_dirty |= is_dirty;
    if (page->pin_count == 0) {
        replacer.insert(page_id);
    }
}

void BufferPoolManager::flushPage(int page_id) {
    std::lock_guard<std::mutex> disk_lock(disk_io_mutex);
    if (!page_table.count(page_id)) return;
    Page* page = page_table[page_id];
    if (page->is_dirty) {
        writePageToDisk(page_id, page->data);
        page->is_dirty = false;
    }
}

void BufferPoolManager::flushAllPages() {
    std::lock_guard<std::mutex> lock(page_table_mutex);
    for (auto& [pid, page] : page_table) {
        flushPage(pid);
    }
}

void BufferPoolManager::shutdown() {
    flushAllPages();
    closeDiskFile();
}

// // Test main function
// int main() {
//     BufferPoolManager bpm(3); // Buffer pool with 3 frames
//     std::string table_name = "test_table";

//     // Allocate and write to 3 pages
//     for (int logical_page_number = 0; logical_page_number < 3; ++logical_page_number) {
//         allocatePage(table_name, logical_page_number);
//         Page* page = bpm.fetchPage(table_name, logical_page_number);
//         if (!page) {
//             std::cerr << "Failed to fetch page " << logical_page_number << "\n";
//             return 1;
//         }
//         std::string content = "Hello Page " + std::to_string(logical_page_number);
//         strncpy(page->data, content.c_str(), PAGE_SIZE - 1);
//         page->data[PAGE_SIZE - 1] = '\0'; // Ensure null-termination
//         bpm.unpinPage(page->page_id, true);
//     }

//     // Allocate a 4th page, which should evict an unpinned page
//     int logical_page_number = 3;
//     allocatePage(table_name, logical_page_number);
//     Page* evicted_page = bpm.fetchPage(table_name, logical_page_number);
//     if (!evicted_page) {
//         std::cerr << "Failed to fetch page " << logical_page_number << "\n";
//         return 1;
//     }
//     strncpy(evicted_page->data, "Evicted someone!", PAGE_SIZE - 1);
//     evicted_page->data[PAGE_SIZE - 1] = '\0';
//     bpm.unpinPage(evicted_page->page_id, true);

//     // Read back all pages
//     for (int i = 0; i <= logical_page_number; ++i) {
//         Page* page = bpm.fetchPage(table_name, i);
//         if (!page) {
//             std::cerr << "Failed to fetch page " << i << "\n";
//             continue;
//         }
//         std::cout << "Page " << i << " data: " << page->data << "\n";
//         bpm.unpinPage(page->page_id, false);
//     }

//     bpm.shutdown();
//     return 0;
// }