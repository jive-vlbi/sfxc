/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: Data_writer_void.cc 251 2007-06-12 13:56:30Z kruithof $
 *
 */

#include "data_writer_void.h"

Data_writer_void::Data_writer_void() : Data_writer() {}

Data_writer_void::~Data_writer_void() {}

size_t
Data_writer_void::do_put_bytes(size_t nBytes, const char *) {
  return nBytes;
}
