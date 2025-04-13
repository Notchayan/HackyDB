#include "log_manager.hpp"

LogManager::LogManager(const std::string& filename) {
    log_file.open(filename, std::ios::app | std::ios::binary);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open WAL log file\n";
        exit(1);
    }
}

LogManager::~LogManager() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void LogManager::logWrite(int page_id, const char* data) {
    log_file.write(reinterpret_cast<char*>(&page_id), sizeof(int));
    log_file.write(data, PAGE_SIZE);
    log_file.flush();
}

void LogManager::recover(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "WAL log file not found for recovery.\n";
        return;
    }

    int pid;
    char buffer[PAGE_SIZE];
    while (in.read(reinterpret_cast<char*>(&pid), sizeof(int))) {
        in.read(buffer, PAGE_SIZE);
        writePageToDiskStatic(pid, buffer);
    }

    in.close();
}

void LogManager::writePageToDiskStatic(int page_id, const char* data) {
    int fd = open("dbfile", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        std::cerr << "Failed to open dbfile for recovery\n";
        return;
    }
    lseek(fd, page_id * PAGE_SIZE, SEEK_SET);
    write(fd, data, PAGE_SIZE);
    close(fd);
}
