/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_FILE_H
#define DATA_READER_FILE_H

#include <Data_reader.h>
#include <vector>
#include <fstream>

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

  bool eof();  
  
private:
  size_t do_get_bytes(size_t nBytes, char *out);

  std::ifstream file;
//  std::vector<char> buffer;
};

#endif // DATA_READER_FILE_H
