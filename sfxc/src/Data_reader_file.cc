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

Data_reader_file::~Data_reader_file()
{
  close(file);
}

UINT64 Data_reader_file::move_forward(UINT64 nBytes)
{
  std::cout << "MOVE_FORWARD is deprecated" << std::endl;
  return 0;
}

UINT64 Data_reader_file::get_bytes(UINT64 nBytes, char*out)
{
//  if (nBytes != 80000) 
//    std::cout << "get_bytes " << nBytes << std::endl;
  if (out == NULL) {
    UINT64 pos = lseek(file, 0, SEEK_CUR);
    return lseek(file, nBytes, SEEK_CUR) - pos;
  }
  return read(file, out, nBytes);
}
