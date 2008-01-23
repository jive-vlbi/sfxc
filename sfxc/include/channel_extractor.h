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
#include "tasklet/tasklet.h"
#include "input_node_types.h"
#include "control_parameters.h"

// Not a data reader since it outputs multiple streams
template <class Type>
class Channel_extractor : public Tasklet {
public:
  typedef Input_node_types<Type>                        Types;

  typedef typename Types::Fft_buffer                    Input_buffer;
  typedef typename Input_buffer::value_type             Input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>               Input_buffer_ptr;

  typedef typename Types::Channel_memory_pool           Output_memory_pool;
  typedef typename Types::Channel_buffer                Output_buffer;
  typedef typename Output_buffer::value_type            Output_buffer_element;
  typedef boost::shared_ptr<Output_buffer>              Output_buffer_ptr;

  Channel_extractor();
  virtual ~Channel_extractor();

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
  // Dechannelize samples
  void process_samples(Type *input_buffer,
                       int n_input_samples,
                       int start_track_nr,
                       int end_track_nr,
                       std::vector<char *> &output_positions,
                       int &output_buffer_bit,
                       int &output_buffer_byte);
private:
  Input_buffer_ptr                input_buffer_;
  Output_memory_pool              output_memory_pool_;
  std::vector<Output_buffer_ptr>  output_buffers_;

  /// Bit positions for the sign and magnitude bits, per channel
  std::vector< std::vector<int> > tracks_in_subbands;
};

#include "channel_extractor_impl.h"

#endif /*CHANNEL_EXTRACTOR_H_*/
