

#ifndef HackyDb_FILE_HANDLE_H_
#define HackyDb_FILE_HANDLE_H_

#include <string>


#include "../../Block/Block_info/block_info.h"
#include "../File_info/file_info.h"

class FileHandle {
private:
  FileInfo *first_file_;
  std::string path_;

public:
  FileHandle(std::string p) : first_file_(new FileInfo()), path_(p) {}
  ~FileHandle();
  FileInfo *GetFileInfo(std::string db_name, std::string tb_name,
                        int file_type);
  BlockInfo *GetBlockInfo(FileInfo *file, int block_pos);
  void AddBlockInfo(BlockInfo *block);
  void IncreaseAge();
  BlockInfo *RecycleBlock();
  void AddFileInfo(FileInfo *file);
  void WriteToDisk();
};

#endif /* defined(HackyDb_FILE_HANDLE_H_) */
