#include "Record_Manager.hpp"

#include <iomanip>
#include <iostream>

#include "IndexManager/IndexManager.hpp"

using namespace std;

BlockInfo *RecordManager::GetBlockInfo(Table *table, int blockNumber) {
    if (blockNumber == -1) {
        return nullptr;
    }
    BlockInfo *blockInfo = hdl_->GetFileBlock(db_name_, table->tb_name(), 0, blockNumber);
    return blockInfo;
}
