/* Copyright (c) 2007 JIVE (Netherlands)
 * All rights reserved.
 * 
 * This file is part of sfxc (software FX correlator);
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef DATA_WRITER_BUFFER_H
#define DATA_WRITER_BUFFER_H

#include <Data_writer.h>
#include <Buffer.h>

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

  UINT64 put_bytes(UINT64 nBytes, char *buff);
private:
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
