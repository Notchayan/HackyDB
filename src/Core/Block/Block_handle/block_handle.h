
#ifndef HackyDb_BLOCK_HANDLE_H_
#define HackyDb_BLOCK_HANDLE_H_

#include "../Block_info/block_info.h"

class BlockHandle {
private:
  BlockInfo *first_block_;
  int bsize_;  // total #
  int bcount_; // usable #
  std::string path_;

  // Inits BlockHandle
  BlockInfo *Add(BlockInfo *block);

public:
  BlockHandle(std::string p)
      : first_block_(new BlockInfo(0)), bsize_(300), bcount_(0), path_(p) {
    Add(first_block_);
  }

  ~BlockHandle();

  int bcount() { return bcount_; }

  BlockInfo *GetUsableBlock();

  void FreeBlock(BlockInfo *block);
};

#endif /* defined(HackyDb_BLOCK_HANDLE_H_) */
