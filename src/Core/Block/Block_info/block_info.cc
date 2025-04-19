#include "block_info.h"

#include <fstream>   // For file read/write
#include "../../../Includes/commons.h" // Contains shared constants like FORMAT_INDEX, etc.

using namespace std;

// Reads block data from disk into memory
void BlockInfo::ReadInfo(std::string path) {
  // Construct the full path based on database and file name
  path += file_->db_name() + "/" + file_->file_name();

  // Add appropriate file extension based on the file type
  if (file_->type() == FORMAT_INDEX) {
    path += ".index";   // Index file
  } else {
    path += ".records"; // Data records file
  }

  // Open file in binary mode for reading
  ifstream ifs(path, ios::binary);

  // Move the read pointer to the correct offset for this block
  ifs.seekg(block_num_ * 4 * 1024); // Each block is 4KB

  // Read 4KB of block data into memory buffer `data_`
  ifs.read(data_, 4 * 1024);

  // Close the file
  ifs.close();
}

// Writes block data from memory back to disk
void BlockInfo::WriteInfo(std::string path) {
  // Construct the full path based on database and file name
  path += file_->db_name() + "/" + file_->file_name();

  // Add appropriate file extension based on the file type
  if (file_->type() == FORMAT_INDEX) {
    path += ".index";   // Index file
  } else {
    path += ".records"; // Data records file
  }

  // Open file in binary mode for writing
  ofstream ofs(path, ios::binary);

  // Move the write pointer to the correct offset for this block
  ofs.seekp(block_num_ * 4 * 1024); // Each block is 4KB

  // Write 4KB of block data from memory buffer `data_` to disk
  ofs.write(data_, 4 * 1024);

  // Close the file
  ofs.close();
}
