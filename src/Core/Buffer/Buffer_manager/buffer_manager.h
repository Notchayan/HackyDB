

#ifndef HackyDb_BUFFER_MANAGER_H_
#define HackyDb_BUFFER_MANAGER_H_

#include <string>

#include "../../Block/Block_handle/block_handle.h"
#include "../../File/File_handle/file_handle.h"

class BufferManager {
private:
  BlockHandle *bhandle_;
  FileHandle *fhandle_;
  std::string path_;

  BlockInfo *GetUsableBlock();

public:
  BufferManager(std::string p)
      : bhandle_(new BlockHandle(p)), fhandle_(new FileHandle(p)), path_(p) {}
  ~BufferManager() {
    delete bhandle_;
    delete fhandle_;
  }

  BlockInfo *GetFileBlock(std::string db_name, std::string tb_name,
                          int file_type, int block_num);
  void WriteBlock(BlockInfo *block);
  void WriteToDisk();
};

#endif /* defined(HackyDb_HANDLE_H_) */
