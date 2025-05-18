#include "file_stream.h"
#include <iostream>

using namespace std;

auto FileStream::loadBytes(size_t offset, size_t size) -> unique_ptr<char[]> {
  auto uni = make_unique<char[]>(size);
  seekg(offset, ios::beg);
  read(uni.get(), size);
  if (gcount() != (streamsize)size) {
    cout << "can't read enough bytes\n";
    if (gcount() == 0) {
      return nullptr;
    }
  }
  return uni;
}
