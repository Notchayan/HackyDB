

#include <string>

#include "../BlockHandle/BlockHandle.hpp"
#include "../FileHandle/FileHandle.hpp"

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
