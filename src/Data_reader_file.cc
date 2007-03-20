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
  Data_reader()
{
  file.open(filename, std::ios::in | std::ios::binary);
  assert(file.is_open() );
}

Data_reader_file::~Data_reader_file() {
  file.close();
}

UINT64 Data_reader_file::get_bytes(UINT64 nBytes, char*out) {
  if (out == NULL) {
    UINT64 pos = file.tellg();
    file.seekg (nBytes, std::ios::cur);
    UINT64 pos2 = file.tellg();
    return pos2 - pos;
  }
  file.read(out, nBytes);
  if (file.eof()) return file.gcount();
  return nBytes;
}

bool Data_reader_file::eof() {
  return file.eof(); 
}
