#ifndef __FILE_STREAM_H__
#define __FILE_STREAM_H__

#include <fstream>
#include <memory>

using namespace std;

class FileStream : fstream {
public:
  FileStream(const char *path,
             ios_base::openmode mode = ios_base::binary | ios_base::in)
      : fstream(path, mode) {}

  template <class Block>
  auto loadBlock(Block &block, size_t offset = 0, size_t index = 0) {
    seekg(offset + sizeof(block) * index, ios::beg);
    read(reinterpret_cast<char *>(&block), sizeof(block));
    return gcount();
  }

  template <class Block>
  auto loadBlocks(Block *block, size_t offset, size_t num) {
    seekg(offset, ios::beg);
    read(reinterpret_cast<char *>(block), num * sizeof(*block));
    return gcount();
  }

  auto loadBytes(size_t offset, size_t size) -> unique_ptr<char[]>;
};
#endif
