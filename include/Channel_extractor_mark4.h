/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Buffer.h 191 2007-04-05 11:34:41Z kruithof $
 *
 */

#ifndef MARK4_CHANNEL_EXTRACTOR_H_
#define MARK4_CHANNEL_EXTRACTOR_H_

#include <Channel_extractor.h>
#include <Data_writer.h>

#include <staPrms.h>

#include <vector>

// Templated by the type of the element from which the samples are extracted
// Either INT32 (n_head_stacks == 1) or INT64 (n_head_stacks == 2)
template <class T>
class Channel_extractor_mark4_implementation;

class Channel_extractor_mark4 : public Channel_extractor
{
public:
  enum DEBUG_LEVEL {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };

  Channel_extractor_mark4(Data_reader &reader, 
                          StaP &StaPrms, 
                          DEBUG_LEVEL debug_level = CHECK_PERIODIC_HEADERS);
  
  int goto_time(INT64 time);
  INT64 get_current_time();

  bool eof();
private:
  size_t do_get_bytes(size_t nBytes, char *buff);
  
  /** The number of head stacks
   * (determines whether one sample of channels is 32 or 64 bits)
   **/
  int n_head_stacks;
  
  // Different implementations based on the number of head stacks:
  Channel_extractor_mark4_implementation<UINT32> *ch_extractor_1_head_stack;
  Channel_extractor_mark4_implementation<UINT64> *ch_extractor_2_head_stack;
};

#endif /*MARK4_CHANNEL_EXTRACTOR_H_*/
