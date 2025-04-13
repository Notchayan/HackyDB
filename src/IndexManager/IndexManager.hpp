#ifndef INDEX_MANAGER_HPP
#define INDEX_MANAGER_HPP

#include <string>
#include <map>
#include <optional>

class IndexManager {
public:
    // Each key maps to (page_id, row_offset)
    using RowLocation = std::pair<int, int>;

private:
    std::map<std::string, RowLocation> index;

public:
    // Insert a key and its row location into the index
    bool insertKey(const std::string& key, int page_id, int row_offset);

    // Find a key and return its row location, if exists
    std::optional<RowLocation> findKey(const std::string& key);
};

#endif // INDEX_MANAGER_HPP
