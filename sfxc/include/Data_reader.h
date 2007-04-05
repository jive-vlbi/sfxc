/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_H
#define DATA_READER_H


#include <types.h>
#include <iostream>

/** Virtual class defining the interface for obtaining input.
 **/
class Data_reader {
public:
  virtual ~Data_reader() {
  }

  /** Reads nBytes from channel into the buff starting from the
      read_pointer. If buff == NULL, then the buffer is not 
      filled and the read pointer is only increased by the amount of bytes.
      
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/ 
  virtual UINT64 get_bytes(UINT64 nBytes, char *buff) = 0;

  /** Returns true if all data is read from the input reader.
  **/ 
  virtual bool eof() = 0;
};


#endif // DATA_READER_H
