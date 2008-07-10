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
  int get_bytes(size_t nBytes, char *buff);

  /** Returns true if all data is read from the input reader.
  **/
  virtual bool eof() = 0;

  /** Returns the number of bytes written
   **/
  uint64_t data_counter();

  /** Resets the number of bytes written
   **/
  void reset_data_counter();

  /** Sets the size of the data slice to read.
      - -1: Don't use the dataslice counter
      - 0: End of data slice
      - >0: Number of bytes to read in the current dataslice
   **/
  void set_size_dataslice(int data_size);
  /** Gets the size of the data slice to read.
      - -1: The dataslice counter is not in use
      - 0: End of data slice
      - >0: Number of bytes to read in the current dataslice
   **/
  int get_size_dataslice();

  /** returns true if eof or the end of the data slice has been reached **/
  bool end_of_dataslice();

  /** returns true if at least one byte can be read **/
  virtual bool can_read() = 0;

  virtual int get_fd() {
    return -1;
  }

private:
  /** Function that actually writes the data to the output device.
  **/
  virtual int do_get_bytes(size_t nBytes, char *buff) = 0;

  uint64_t _data_counter;
  int data_slice;
};


#endif // DATA_READER_H
