/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

#include <Data_reader_file.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <fcntl.h> // file control

Data_reader_file::Data_reader_file(char *filename) : 
  Data_reader(), file(-1)
{
  file = open(filename, O_RDONLY, 0);
  assert(file >= 0);
}

Data_reader_file::~Data_reader_file() {
  close(file);
}

UINT64 Data_reader_file::move_forward(UINT64 nBytes) {
  std::cout << "MOVE_FORWARD is deprecated" << std::endl;
//   assert(nBytes >= 0);
//   if (buffer.size() > nBytes) {
//     assert(nBytes < buffer.capacity());
//     buffer.erase(buffer.begin(), buffer.begin()+nBytes);
//   } else {
//     // Completely empty the buffer and forward the filepointer.
//     lseek(file, nBytes-buffer.size(), SEEK_CUR);
//     buffer.clear();
//   }
//   return nBytes;
  return 0;
}

UINT64 Data_reader_file::get_bytes(UINT64 nBytes, char*out) {
  if (out == NULL) {
    UINT64 pos = lseek(file, 0, SEEK_CUR);
    return lseek(file, nBytes, SEEK_CUR) - pos;
  }
  return read(file, out, nBytes);
//   if (nBytes > buffer.capacity()) {
//     buffer.reserve(nBytes);
//   }
//   if (nBytes > buffer.size()) {
//     // always completely fill the buffer
//     INT64 nRead = buffer.capacity() - buffer.size();
//     char *tmp_buff = new char[nRead];

//     std::vector<char>::iterator it = buffer.end();
//     buffer.resize(buffer.size()+size);
//     std::copy(tmp_buff, tmp_buff + size, it);

//     delete[] tmp_buff;
//   }

//   std::copy(buffer.begin(), 
// 	    buffer.begin() + std::min(buffer.size(), (size_t)nBytes),
// 	    out);
//   nBytes = std::min(buffer.size(), (size_t)nBytes);
//   move_forward(nBytes);
//   return nBytes;
}
