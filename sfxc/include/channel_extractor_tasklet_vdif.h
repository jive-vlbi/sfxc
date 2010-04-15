/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema, 2010
 *
 * $Id: channel_extractor_tasklet_vdif.h 888 2008-08-28 09:48:03Z marchal $
 *
 */

#ifndef CHANNEL_EXTRACTOR_TASKLET_VDIF_H_
#define CHANNEL_EXTRACTOR_TASKLET_VDIF_H_

#include <vector>

#include "utils.h"
#include "tasklet/tasklet.h"
#include "thread.h"
#include "input_node_types.h"
#include "control_parameters.h"
#include "channel_extractor_tasklet.h"

#include "channel_extractor_interface.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif //RUNTIME_STATISTIC

#include "timer.h"

#define MAX_SUBBANDS 16

/**
 * The channel extractor gets a chunk of data and outputs the dechannelized data
 **/
class Channel_extractor_tasklet_VDIF :  public Channel_extractor_tasklet{
public:
  /**
   * Constructor
   * \param samples_per_block Number of input words to process
   * \param N                 Number of bytes per input word
   **/
  Channel_extractor_tasklet_VDIF(int samples_per_block, int N);

  virtual ~Channel_extractor_tasklet_VDIF();
  /// Process one piece of data
  virtual void do_task();
};

#endif /*CHANNEL_EXTRACTOR_TASKLET_VDIF_H_*/
