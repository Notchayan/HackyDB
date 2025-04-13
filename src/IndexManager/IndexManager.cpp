#include "IndexManager.hpp"

bool IndexManager::insertKey(const std::string& key, int page_id, int row_offset) {
    if (index.find(key) != index.end()) {
        return false; // Duplicate key not allowed for now
    }
    index[key] = {page_id, row_offset};
    return true;
}

std::optional<IndexManager::RowLocation> IndexManager::findKey(const std::string& key) {
    auto it = index.find(key);
    if (it != index.end()) {
        return it->second;
    }
    return std::nullopt;
}
