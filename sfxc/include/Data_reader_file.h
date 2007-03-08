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

#ifndef DATA_READER_FILE_H
#define DATA_READER_FILE_H

#include <Data_reader.h>
#include <vector>

//

/** Specialisation of Data_reader for reading files from a linux
    filesystem.
 **/
class Data_reader_file : public Data_reader {
  public:
  /** Constructor, reads from file
   **/
  Data_reader_file(char * filename);

  ~Data_reader_file();

  UINT64 move_forward(UINT64 nBytes);
  UINT64 get_bytes(UINT64 nBytes, char *out);
  
private:
  int file;
  std::vector<char> buffer;
};

#endif // DATA_READER_FILE_H
