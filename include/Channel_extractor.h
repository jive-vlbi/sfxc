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

#include <utils.h>
#include <vector>

// Not a data reader since it outputs multiple streams
class Channel_extractor
{
public:
  Channel_extractor() {
  }

  virtual ~Channel_extractor() {
  }

  virtual int goto_time(int64_t time) = 0;
  virtual int64_t get_current_time() = 0;

  /** Returns a number of samples, one sample per character. **/
  virtual size_t get_bytes(std::vector< char * > &buff)=0;

};

#endif /*CHANNEL_EXTRACTOR_H_*/
