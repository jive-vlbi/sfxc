/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_BUFFER_H
#define DATA_READER_BUFFER_H

#include <Data_reader.h>
#include <Buffer.h>

//

/** Specialisation of Data_reader for reading from a buffer.
 **/
class Data_reader_buffer : public Data_reader {
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  
public:
  /** Constructor, reads from buffer
   **/
  Data_reader_buffer(Buffer *buff);

  ~Data_reader_buffer();

  UINT64 get_bytes(UINT64 nBytes, char *out);

  bool eof();  
  
private:
  // The input buffer
  Buffer       *buffer;
  // Number of bytes left in the current buffer-element
  int          bytes_left;
  // element: pointer to the beginning of the buffer
  // data_start: current read pointer
  //value_type   *element;
  char *data_start;
  // Is there more data arriving in the buffer:
  bool         end_of_file;
};

#endif // DATA_READER_BUFFER_H
