/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Data_writer_void.h 251 2007-06-12 13:56:30Z kruithof $
 *
 */

#ifndef DATA_WRITER_VOID_H
#define DATA_WRITER_VOID_H

#include "data_writer.h"

class Data_writer_void : public Data_writer {
public:
  Data_writer_void();
  ~Data_writer_void();
  
  size_t do_put_bytes(size_t nBytes, const char *buff);
};

#endif // DATA_WRITER_VOID_H
