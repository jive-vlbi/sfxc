/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef CHANNEL_EXTRACTOR_TASKLET_H_
#define CHANNEL_EXTRACTOR_TASKLET_H_

#include <vector>

#include "utils.h"
#include "tasklet/tasklet.h"
#include "input_node_types.h"
#include "control_parameters.h"

#include "channel_extractor_interface.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif //RUNTIME_STATISTIC


#define MAX_SUBBANDS 16

// Not a data reader since it outputs multiple streams
class Channel_extractor_tasklet : public Tasklet {
public:
  typedef Input_node_types                     Types;

  typedef Types::Mk4_buffer                    Input_buffer;
  typedef Input_buffer::value_type             Input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>      Input_buffer_ptr;

  typedef Types::Channel_memory_pool           Output_memory_pool;
  typedef Types::Channel_buffer                Output_buffer;
  typedef Types::Channel_buffer_element        Output_buffer_element;
  typedef boost::shared_ptr<Output_buffer>     Output_buffer_ptr;

  Channel_extractor_tasklet(int N);
  virtual ~Channel_extractor_tasklet();

  /// For tasklet

  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  const char *name() {
    return "Channel_extractor";
  }

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);
  /// Get the output
  Output_buffer_ptr get_output_buffer(size_t stream);

  // Setting parameters
  void set_parameters(const Input_node_parameters &input_node_param,
                      const std::vector< std::vector<int> > &track_positions);

private:
  Input_buffer_ptr                input_buffer_;
  Output_memory_pool              output_memory_pool_;
  std::vector<Output_buffer_ptr>  output_buffers_;

  Channel_extractor_interface     *ch_extractor;

  // Cached values
  size_t n_subbands;      // Number of subbands
  size_t fan_out;         // Number of output bits per input sample
  size_t bits_per_sample; // Number of bits per output sample

  // Size of one input word in bytes (#tracks/8)
  const int N;

#ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed monitor_;
#endif //RUNTIME_STATISTIC
};

#include "channel_extractor_impl.h"

#endif /*CHANNEL_EXTRACTOR_TASKLET_H_*/
