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

class BufferPoolManager {
    int pool_size;
    std::unordered_map<int, Page*> page_table;
    std::vector<Page*> pages;
    std::unordered_map<int, std::string> disk_file_map;
    LRUReplacer replacer;

public:
    BufferPoolManager(int size): pool_size(size) {
        for (int i = 0; i < pool_size; ++i)
            pages.push_back(new Page());
    }

    ~BufferPoolManager() {
        for (auto page : pages) {
            flushPage(page->page_id);
            delete page;
        }
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

    void readPageFromDisk(int page_id, char* data) {
        std::ifstream in("dbfile", std::ios::binary);
        in.seekg(page_id * PAGE_SIZE);
        in.read(data, PAGE_SIZE);
        in.close();
    }

    void writePageToDisk(int page_id, char* data) {
        std::ofstream out("dbfile", std::ios::binary | std::ios::in | std::ios::out);
        out.seekp(page_id * PAGE_SIZE);
        out.write(data, PAGE_SIZE);
        out.close();
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

