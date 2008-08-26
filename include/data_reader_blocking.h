/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *  This file contains:
 *     - the declaration of a very simple blocking data
 *       data reader.
 */
#ifndef DATA_READER_BLOCKING_H_INCLUDED
#define DATA_READER_BLOCKING_H_INCLUDED

#include "data_reader.h"

#include <unistd.h>

/****************************************************
*
* @class Data_reader_blocking
* @author Damien Marchal
* @desc A data_reader offer no guarantee that the
* the amount of byte read is equal to the number of
* byte requested to read. So you can receive half the
* data you asked for. The data_reader_blocking do
* a while loop around this situation. The only reason
* why the number of byte read is != to the number of
* byte requested is when the under-lying data reader
* is closed/eof().
*
****************************************************/
class Data_reader_blocking : public Data_reader {
  Data_reader* m_reader;
public:
  Data_reader_blocking(Data_reader *rdr);

  bool eof();
  bool can_read();

  static int get_bytes_s(Data_reader* reader, size_t size, char* buffer);
  static int get_bytes_s(Data_reader* reader, size_t size, char* buffer, int scaling);

private:
  int do_get_bytes(size_t size, char* buffer);
};

Data_reader_blocking& operator>>(Data_reader_blocking& dr, std::string& str);
Data_reader_blocking& operator>>(Data_reader_blocking& dr, uint32_t& value);
Data_reader_blocking& operator>>(Data_reader_blocking& dr, int32_t& value);

#endif // DATA_READER_BLOCKING_H_INCLUDED
