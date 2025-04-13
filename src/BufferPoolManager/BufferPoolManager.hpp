#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#include "../Log_manager/log_manager.hpp"
#include "../constants.hpp"

struct Page {
    int page_id = -1;
    bool is_dirty = false;
    int pin_count = 0;
    char data[PAGE_SIZE];

    Page();
};

class LRUReplacer {
    std::list<int> lru_list;
    std::unordered_map<int, std::list<int>::iterator> map;
    std::mutex replacer_mutex;

public:
    void insert(int page_id);
    void erase(int page_id);
    bool victim(int& page_id);
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
    BufferPoolManager(int size);
    ~BufferPoolManager();

    Page* fetchPage(int page_id);
    Page* fetchPage(const std::string& table_name, int logical_page_number);

    void unpinPage(int page_id, bool is_dirty);
    void flushPage(int page_id);
    void flushAllPages();
    void shutdown();
};

// Global functions and variables
extern int next_page_id;
extern int disk_fd;
extern int next_physical_page_id;
extern std::unordered_map<std::string, std::unordered_map<int, int>> page_directory;

extern std::mutex page_directory_mutex;
extern std::mutex physical_page_mutex;
extern std::mutex disk_io_mutex;

void loadMetadata();
void saveMetadata();
void openDiskFile();
void closeDiskFile();
void readPageFromDisk(int page_id, char* data);
void writePageToDisk(int page_id, char* data);
void mapPage(const std::string& table_name, int logical_page_number, int physical_page_id);
int allocatePage(const std::string& table_name, int logical_page_number);
