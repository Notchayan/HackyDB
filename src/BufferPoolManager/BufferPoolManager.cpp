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

const int PAGE_SIZE = 4096;

struct Page {
    int page_id = -1;
    bool is_dirty = false;
    int pin_count = 0; // Prevent eviction if > 0
    char data[PAGE_SIZE];

    Page() {
        memset(data, 0, PAGE_SIZE);
    }
};

class LRUReplacer {
    std::list<int> lru_list;
    std::unordered_map<int, std::list<int>::iterator> map;

public:
    void insert(int page_id) {
        if (map.count(page_id)) {
            lru_list.erase(map[page_id]);
        }
        lru_list.push_front(page_id);
        map[page_id] = lru_list.begin();
    }

    void erase(int page_id) {
        if (map.count(page_id)) {
            lru_list.erase(map[page_id]);
            map.erase(page_id);
        }
    }

    bool victim(int &page_id) {
        if (lru_list.empty()) return false;
        page_id = lru_list.back();
        lru_list.pop_back();
        map.erase(page_id);
        return true;
    }
};

int disk_fd = -1; // File descriptor

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


class BufferPoolManager {
    int pool_size;
    std::unordered_map<int, Page*> page_table;
    std::vector<Page*> pages;
    std::unordered_map<int, std::string> disk_file_map;
    LRUReplacer replacer;

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
        if (page_table.count(page_id)) {
            Page* page = page_table[page_id];
            page->pin_count++;
            replacer.erase(page_id);
            return page;
        }

        int victim_id;
        Page* frame = nullptr;

        // Find empty frame or evict one
        for (auto p : pages) {
            if (p->page_id == -1) {
                frame = p;
                break;
            }
        }

        if (!frame) {
            if (!replacer.victim(victim_id)) return nullptr;
            frame = page_table[victim_id];
            if (frame->is_dirty)
                flushPage(victim_id);
            page_table.erase(victim_id);
        }

        // Load from disk
        frame->page_id = page_id;
        frame->pin_count = 1;
        frame->is_dirty = false;
        readPageFromDisk(page_id, frame->data);
        page_table[page_id] = frame;

        return frame;
    }

    void unpinPage(int page_id, bool is_dirty) {
        if (!page_table.count(page_id)) return;
        Page* page = page_table[page_id];
        if (page->pin_count > 0) page->pin_count--;
        page->is_dirty |= is_dirty;
        if (page->pin_count == 0)
            replacer.insert(page_id);
    }

    void flushPage(int page_id) {
        if (!page_table.count(page_id)) return;
        Page* page = page_table[page_id];
        if (page->is_dirty)
            writePageToDisk(page_id, page->data);
        page->is_dirty = false;
    }
};



int main() {
    BufferPoolManager bpm(3); // 3-page buffer pool

    Page* p1 = bpm.fetchPage(0);
    strcpy(p1->data, "Hello Page 0");
    bpm.unpinPage(0, true);

    Page* p2 = bpm.fetchPage(1);
    strcpy(p2->data, "Hello Page 1");
    bpm.unpinPage(1, true);

    Page* p3 = bpm.fetchPage(2);
    strcpy(p3->data, "Hello Page 2");
    bpm.unpinPage(2, true);

    // This will cause eviction
    Page* p4 = bpm.fetchPage(3);
    strcpy(p4->data, "Evicted someone!");
    bpm.unpinPage(3, true);

    return 0;
}

