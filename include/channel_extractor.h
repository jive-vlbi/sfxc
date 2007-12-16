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

#include <vector>

#include "utils.h"
#include "tasklet/Tasklet.h"

// Not a data reader since it outputs multiple streams
template <class Type>
class Channel_extractor : public Tasklet
{
public:
  Channel_extractor() {
  }
  virtual ~Channel_extractor() {
  }
  
  void do_task();
  
private:
};

#endif /*CHANNEL_EXTRACTOR_H_*/
