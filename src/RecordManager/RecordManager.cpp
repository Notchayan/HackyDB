#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>     // for sanity checks

#include "../BufferPoolManager/BufferPoolManager.hpp"

struct Record {
    int id;
    char name[32];
    int age;
};

class RecordManager {
    BufferPoolManager* bpm;
    std::string table_name;
    int records_per_page = PAGE_SIZE / sizeof(Record);

public:
    RecordManager(BufferPoolManager* bpm, const std::string& table_name)
        : bpm(bpm), table_name(table_name) {}

    int insertRecord(const Record& record) {
        // Try to find space in existing pages
        for (int logical_page = 0; logical_page < 100; ++logical_page) { // fixed for now
            Page* page = bpm->fetchPage(table_name, logical_page);
            if (page == nullptr) {
                allocatePage(table_name, logical_page); // from bufferpool
                page = bpm->fetchPage(table_name, logical_page);
            }

            Record* records = reinterpret_cast<Record*>(page->data);
            for (int i = 0; i < records_per_page; ++i) {
                if (records[i].id == -1) {  // empty slot marker
                    records[i] = record;
                    bpm->unpinPage(page->page_id, true);
                    return logical_page * records_per_page + i;
                }
            }
            bpm->unpinPage(page->page_id, false);
        }
        return -1; // failed to insert
    }

    Record getRecord(int record_id) {
        int logical_page = record_id / records_per_page;
        int slot = record_id % records_per_page;

        Page* page = bpm->fetchPage(table_name, logical_page);
        assert(page != nullptr);
        Record* records = reinterpret_cast<Record*>(page->data);
        Record r = records[slot];
        bpm->unpinPage(page->page_id, false);
        return r;
    }
};
