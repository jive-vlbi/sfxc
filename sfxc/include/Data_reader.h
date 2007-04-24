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
  Data_reader();

  virtual ~Data_reader();

  /** Reads nBytes from channel into the buff starting from the
      read_pointer. If buff == NULL, then the buffer is not 
      filled and the read pointer is only increased by the amount of bytes.
      
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/
  size_t get_bytes(size_t nBytes, char *buff);

  /** Returns true if all data is read from the input reader.
  **/ 
  virtual bool eof() = 0;

  /** Returns the number of bytes written
   **/
  UINT64 data_counter();

  /** Resets the number of bytes written
   **/
  void reset_data_counter();

private:
  /** Function that actually writes the data to the output device.
  **/ 
  virtual size_t do_get_bytes(size_t nBytes, char *buff) = 0;

  UINT64 _data_counter;
};


#endif // DATA_READER_H
