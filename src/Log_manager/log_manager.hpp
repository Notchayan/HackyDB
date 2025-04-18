#pragma once
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "../include/shared_structs_constants.hpp"

class LogManager {
    std::ofstream log_file;

public:
    LogManager(const std::string& filename = "wal.log");
    ~LogManager();

    void logWrite(int page_id, const char* data);
    static void recover(const std::string& filename = "wal.log");
    static void writePageToDiskStatic(int page_id, const char* data);
};
