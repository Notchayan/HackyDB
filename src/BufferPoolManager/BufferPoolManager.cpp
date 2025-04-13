#include <iostream>              // for std::cout, std::cerr
#include <fstream>              // for std::ifstream, std::ofstream (file I/O)
#include <unordered_map>        // for std::unordered_map (hash tables)
#include <list>                 // for std::list (for LRU implementation)
#include <vector>               // for std::vector (page pool)
#include <cstring>              // for std::memset, std::strcpy
#include <thread>               // for std::thread (if doing background I/O)
#include <mutex>                // for std::mutex
#include <condition_variable>   // for std::condition_variable
#include <queue>                // for std::queue (for disk scheduler requests)
#include <fcntl.h>              // for open()
#include <unistd.h>             // for read(), write(), close()
#include <errno.h>              // for errno
#include <cstring>              // for strerror
#include "../Log_manager/log_manager.hpp"
#include "../constants.hpp"

int next_page_id = 0;
int disk_fd = -1; 
std::unordered_map<std::string, std::unordered_map<int, int>> page_directory;

std::mutex page_directory_mutex;
std::mutex physical_page_mutex;
std::mutex disk_io_mutex;


void openDiskFile() {
    disk_fd = open("dbfile", O_RDWR | O_CREAT, 0644);
    if (disk_fd < 0) {
        std::cerr << "Failed to open disk file: " << strerror(errno) << "\n";
        exit(1);
    }
}

void closeDiskFile() {
    if (disk_fd >= 0) close(disk_fd);
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
        memset(data + bytes, 0, PAGE_SIZE - bytes); // pad with zeros
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

struct Page {
    int page_id = -1;
    bool is_dirty = false;
    int pin_count = 0; 
    char data[PAGE_SIZE];

    Page() {
        memset(data, 0, PAGE_SIZE);
    }
};

void mapPage(const std::string& table_name, int logical_page_number, int physical_page_id) {
    page_directory[table_name][logical_page_number] = physical_page_id;
}

int next_physical_page_id = 0; 

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

bool deletePage(const std::string& table_name, int logical_page_number) {
    std::lock_guard<std::mutex> dir_lock(page_directory_mutex);
    if (!page_directory.count(table_name) || 
        !page_directory[table_name].count(logical_page_number)) {
        return false;
    }

    int physical_page_id = page_directory[table_name][logical_page_number];
    page_directory[table_name].erase(logical_page_number);
    
    {
        std::lock_guard<std::mutex> pt_lock(page_table_mutex);
        if (page_table.count(physical_page_id)) {
            Page* page = page_table[physical_page_id];
            if (page->pin_count > 0) return false; // can't delete pinned page
            replacer.erase(physical_page_id);
            page_table.erase(physical_page_id);
            page->page_id = -1; // reset
        }
    }

    char empty_data[PAGE_SIZE] = {0};
    writePageToDisk(physical_page_id, empty_data); // overwrite on disk
    return true;
}


class LRUReplacer {
    std::list<int> lru_list;
    std::unordered_map<int, std::list<int>::iterator> map;

    std::mutex replacer_mutex;
public:
    void insert(int page_id) {
        std::lock_guard<std::mutex> lock(replacer_mutex);
        if (map.count(page_id)) {
            lru_list.erase(map[page_id]);
        }
        lru_list.push_front(page_id);
        map[page_id] = lru_list.begin();
    }

    void erase(int page_id) {
        std::lock_guard<std::mutex> lock(replacer_mutex);
        if (map.count(page_id)) {
            lru_list.erase(map[page_id]);
            map.erase(page_id);
        }
    }

    bool victim(int &page_id) {
        std::lock_guard<std::mutex> lock(replacer_mutex);
        if (lru_list.empty()) return false;
        page_id = lru_list.back();
        lru_list.pop_back();
        map.erase(page_id);
        return true;
    }
};


class BufferPoolManager {
    int pool_size;
    std::unordered_map<int, Page*> page_table;
    std::vector<Page*> pages;
    std::unordered_map<int, std::string> disk_file_map;
    LRUReplacer replacer;

    std::mutex page_table_mutex;
    std::mutex pages_mutex;
    std::mutex fetch_mutex;

public:
    BufferPoolManager(int size): pool_size(size) {
        openDiskFile(); 
        for (int i = 0; i < pool_size; ++i)
            pages.push_back(new Page());
    }

    ~BufferPoolManager() {
        for (auto page : pages) {
            flushPage(page->page_id);
            delete page;
        }
        closeDiskFile();
    }

    Page* fetchPage(int page_id) {
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

        int victim_id;
        Page* frame = nullptr;

        // Find empty frame or evict one
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
            if (!replacer.victim(victim_id)) return nullptr;
            {
                std::lock_guard<std::mutex> lock(page_table_mutex);
                frame = page_table[victim_id];
                if (frame->is_dirty){
                    flushPage(victim_id);
                }
                page_table.erase(victim_id);
            }
            
        }

        // Load from disk
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

    Page* fetchPage(const std::string& table_name, int logical_page_number) {
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
        return fetchPage(physical_page_id); // call the existing one
    }

    
    void unpinPage(int page_id, bool is_dirty) {
        std::lock_guard<std::mutex> lock(page_table_mutex);
        if (!page_table.count(page_id)) return;
        Page* page = page_table[page_id];
        if (page->pin_count > 0) page->pin_count--;
        page->is_dirty |= is_dirty;
        if (page->pin_count == 0)
            replacer.insert(page_id);
    }

    void flushPage(int page_id) {
        // std::lock_guard<std::mutex> lock(page_table_mutex); this was giving deadlock
        std::lock_guard<std::mutex> disk_lock(disk_io_mutex);
        if (!page_table.count(page_id)) return;
        Page* page = page_table[page_id];
        if (page->is_dirty)
            writePageToDisk(page_id, page->data);
        page->is_dirty = false;
    }

    void flushAllPages() {
        for (auto& [pid, page] : page_table) {
            flushPage(pid);
        }
    }

    void shutdown() {
        flushAllPages();
        closeDiskFile();
    }
    
};

int main() {
    BufferPoolManager bpm(3);
    std::string table_name = "test_table";

    for (int logical_page_number = 0; logical_page_number < 3; ++logical_page_number) {
        allocatePage(table_name, logical_page_number); 

        Page* page = bpm.fetchPage(table_name, logical_page_number); 
        std::string content = "Hello Page " + std::to_string(logical_page_number);
        strcpy(page->data, content.c_str());
        bpm.unpinPage(page->page_id, true); 
    }

    int logical_page_number = 3;
    allocatePage(table_name, logical_page_number);

    Page* evicted_page = bpm.fetchPage(table_name, logical_page_number); 
    strcpy(evicted_page->data, "Evicted someone!");
    bpm.unpinPage(evicted_page->page_id, true); 

    for (int i = 0; i <= logical_page_number; ++i) {
        Page* page = bpm.fetchPage(table_name, i);
        std::cout << "Page " << i << " data: " << page->data << "\n";
        bpm.unpinPage(page->page_id, false); 
    }

    bpm.shutdown();
    return 0;
}
