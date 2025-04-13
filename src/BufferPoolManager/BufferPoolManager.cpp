#include "BufferPoolManager.hpp"
#include <cstring>
#include <iostream>

// Global variables
int disk_fd = -1;
int next_physical_page_id = 0;
std::unordered_map<std::string, std::unordered_map<int, int>> page_directory;

std::mutex page_directory_mutex;
std::mutex physical_page_mutex;
std::mutex disk_io_mutex;



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

void loadPageDirectory() {
    std::lock_guard<std::mutex> lock(page_directory_mutex);

    std::vector<char> buffer(PAGE_SIZE * (METADATA_PAGE_END - METADATA_PAGE_START + 1), 0);
    size_t read_offset = 0;
    int page = METADATA_PAGE_START;

    while (read_offset < buffer.size() && page <= METADATA_PAGE_END) {
        readPageFromDisk(page, &buffer[read_offset]);
        read_offset += PAGE_SIZE;
        page++;
    }

    page_directory.clear();
    size_t offset = 0;

    int num_tables = 0;
    memcpy(&num_tables, &buffer[offset], sizeof(int));
    offset += sizeof(int);

    for (int t = 0; t < num_tables; ++t) {
        int name_len = 0;
        memcpy(&name_len, &buffer[offset], sizeof(int));
        offset += sizeof(int);

        std::string table(&buffer[offset], name_len);
        offset += name_len;

        int num_entries = 0;
        memcpy(&num_entries, &buffer[offset], sizeof(int));
        offset += sizeof(int);

        for (int i = 0; i < num_entries; ++i) {
            int logical, physical;
            memcpy(&logical, &buffer[offset], sizeof(int));
            offset += sizeof(int);
            memcpy(&physical, &buffer[offset], sizeof(int));
            offset += sizeof(int);
            page_directory[table][logical] = physical;
        }
    }
}


void loadMetadata() {
    char buffer[PAGE_SIZE];
    readPageFromDisk(METADATA_PAGE_ID, buffer);
    memcpy(&next_physical_page_id, buffer, sizeof(int));

    loadPageDirectory();
}

void openDiskFile() {
    disk_fd = open("dbfile", O_RDWR | O_CREAT, 0644);
    if (disk_fd < 0) {
        std::cerr << "Failed to open disk file: " << strerror(errno) << "\n";
        exit(1);
    }

    loadMetadata();

    if (next_physical_page_id < FIRST_DATA_PAGE_ID) {
        next_physical_page_id = FIRST_DATA_PAGE_ID;
    }
}

void savePageDirectory() {
    std::lock_guard<std::mutex> lock(page_directory_mutex);

    std::vector<char> buffer(PAGE_SIZE * (METADATA_PAGE_END - METADATA_PAGE_START + 1), 0);
    size_t offset = 0;

    int num_tables = page_directory.size();
    memcpy(&buffer[offset], &num_tables, sizeof(int));
    offset += sizeof(int);

    for (const auto& [table, mapping] : page_directory) {
        int table_name_len = table.size();
        memcpy(&buffer[offset], &table_name_len, sizeof(int));
        offset += sizeof(int);
        memcpy(&buffer[offset], table.c_str(), table_name_len);
        offset += table_name_len;

        int num_entries = mapping.size();
        memcpy(&buffer[offset], &num_entries, sizeof(int));
        offset += sizeof(int);

        for (const auto& [logical, physical] : mapping) {
            memcpy(&buffer[offset], &logical, sizeof(int));
            offset += sizeof(int);
            memcpy(&buffer[offset], &physical, sizeof(int));
            offset += sizeof(int);
        }
    }

    int page = METADATA_PAGE_START;
    size_t written = 0;
    while (written < buffer.size() && page <= METADATA_PAGE_END) {
        writePageToDisk(page, &buffer[written]);
        written += PAGE_SIZE;
        page++;
    }
}


void saveMetadata() {
    char buffer[PAGE_SIZE];
    memset(buffer, 0, PAGE_SIZE);
    memcpy(buffer, &next_physical_page_id, sizeof(int));
    writePageToDisk(METADATA_PAGE_ID, buffer);

    savePageDirectory();
}

void closeDiskFile() {
    saveMetadata();
    if (disk_fd >= 0) {
        close(disk_fd);
        disk_fd = -1;
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
