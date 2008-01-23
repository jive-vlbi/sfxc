/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_WRITER_BUFFER_H
#define DATA_WRITER_BUFFER_H

#include "data_writer.h"
#include "buffer.h"

//

/** Specialisation of Data_writer for writing to a buffer.
 **/
class Data_writer_buffer : public Data_writer {
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  
public:
  /** Constructor, reads from buffer
   **/
  Data_writer_buffer(Buffer *buff);

  ~Data_writer_buffer();

private:
  size_t do_put_bytes(size_t nBytes, char *buff);


  // The input buffer
  Buffer       *buffer;
  // Number of bytes left in the current buffer-element
  int          bytes_left;
  // data_start: current read pointer
  char *data_start;
  // Is there more data arriving in the buffer:
  bool         end_of_file;
};

#endif // DATA_WRITER_BUFFER_H
