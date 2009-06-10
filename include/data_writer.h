/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef DATA_WRITER_H
#define DATA_WRITER_H

#include <types.h>
#include <stddef.h> // defines size_t
#include <string>
#include <boost/shared_ptr.hpp>

class Data_writer {
public:
  Data_writer();

  virtual ~Data_writer();

  /** Writes nBytes from buff to the output device.
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/
  size_t put_bytes(size_t nBytes, const char *buff);

  /** Returns the number of bytes written
   **/
  uint64_t data_counter();

  /** Resets the number of bytes written
   **/
  void reset_data_counter();


  /** Sets the size of the data slice to write.
      - -1: Don't use the dataslice counter
      - 0: End of data slice
      - >0: Number of bytes to read in the current dataslice
   **/
  void set_size_dataslice(int data_size);
  /** Gets the size of the data slice to write.
      - -1: The dataslice counter is not in use
      - 0: End of data slice
      - >0: Number of bytes to read in the current dataslice
   **/
  int get_size_dataslice();

  /** returns true if eof or the end of the data slice has been reached **/
  bool end_of_dataslice();

  /** returns whether we can write at least 1 byte **/
  virtual bool can_write() = 0;

  /** Mark the data writer as active (currently writing data), returns false if already active **/
  bool activate();
  void deactivate();
  /** check whether the data writer is currently active **/
  bool is_active();

private:
  /** Function that actually writes the data to the output device.
  **/
  virtual size_t do_put_bytes(size_t nBytes, const char *buff) = 0;

  uint64_t _data_counter;
  int data_slice;
  bool active; // Flag that indicates if data writer is currently in use

};

Data_writer& operator<<(Data_writer& dr, const std::string& str);
Data_writer& operator<<(Data_writer& dr, uint32_t value);
Data_writer& operator<<(Data_writer& dr, int32_t value);

typedef boost::shared_ptr<Data_writer>       Data_writer_sptr;

#endif // DATA_WRITER_H
