/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef CHANNEL_EXTRACTOR_H_
#define CHANNEL_EXTRACTOR_H_

#include <Data_reader.h>

class Channel_extractor : public Data_reader
{
public:
  Channel_extractor() : Data_reader() {
  }
  
  virtual int goto_time(INT64 time) = 0;
  virtual INT64 get_current_time() = 0;
};

#endif /*CHANNEL_EXTRACTOR_H_*/
