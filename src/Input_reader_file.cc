#include <Input_reader_file.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

Input_reader_file::Input_reader_file(char *filename) : 
  Input_reader(), file(filename, std::ios::in | std::ios::binary)
{
  assert(file.is_open());
}

Input_reader_file::~Input_reader_file() {
  file.close();
}

INT64 Input_reader_file::move_forward(INT64 nBytes) {
  assert(nBytes >= 0);
  if (buffer.size() > nBytes) {
    assert(nBytes < buffer.capacity());
    buffer.erase(buffer.begin(), buffer.begin()+nBytes);
  } else {
    // Completely empty the buffer and forward the filepointer.
    file.seekg(nBytes-buffer.size(), std::ios_base::cur);
    buffer.clear();
  }
  return nBytes;
}

INT64 Input_reader_file::get_bytes(INT64 nBytes, char*out) {
  if (nBytes > buffer.capacity()) {
    buffer.reserve(nBytes);
  }
  if (nBytes > buffer.size()) {
    // always completely fill the buffer
    INT64 nRead = buffer.capacity() - buffer.size();
    char *tmp_buff = new char[nRead];
    INT64 size = file.readsome(tmp_buff, nRead);

    std::vector<char>::iterator it = buffer.end();
    buffer.resize(buffer.size()+size);
    std::copy(tmp_buff, tmp_buff + size, it);

    delete[] tmp_buff;
  }

  std::copy(buffer.begin(), 
	    buffer.begin() + std::min(buffer.size(), (size_t)nBytes),
	    out);
  return std::min(buffer.size(), (size_t)nBytes);
}
