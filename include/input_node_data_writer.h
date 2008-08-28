/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - The declaration of the Input_node_data_writer object. This is object
 *       is used to stream data to the correlation nodes. The object is not
 *       thread safe an thus can only use one client.
 */
#ifndef INPUT_NODE_DATA_WRITER_H_INCLUDED
#define INPUT_NODE_DATA_WRITER_H_INCLUDED

#include <boost/shared_ptr.hpp>
#include "data_writer.h"
#include "utils.h"
#include "timer.h"
#include "rttimer.h"
#include "input_node_types.h"
#include "control_parameters.h"

/// Forward declaration
class Input_node_data_writer;

/// A smart pointer to this object
typedef boost::shared_ptr<Input_node_data_writer> Input_node_data_writer_sptr;

class Input_node_data_writer
{
public:
  typedef Input_node_types::Fft_buffer         Input_buffer;
  typedef Input_node_types::Fft_buffer_element Input_buffer_element;
  typedef Input_node_types::Fft_buffer_ptr     Input_buffer_ptr;

  // The writer can, in principle, occur multiple times in the writer_queue
  // so we store the slice size in a separate integer and set
  // set_size_dataslice(slice_size) when we start writing
  struct Writer_struct {
    Data_writer_sptr writer;
    int             slice_size;
  };
  typedef Threadsafe_queue< Writer_struct >      Data_writer_queue;

  Input_node_data_writer();
  virtual ~Input_node_data_writer();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  void add_timeslice(Data_writer_sptr data_writer, int nr_bytes);

	/// return the amount of data sent...
  uint64_t do_task();

  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters(const Input_node_parameters &input_param);

  /// Empty the input queue, called from the destructor of Input_node
  void empty_input_queue();

  /// Allocate a new shared pointer to this object.
  static Input_node_data_writer_sptr new_sptr();

private:
  Input_buffer_ptr    input_buffer_;
  Data_writer_queue    data_writers_;

  uint64_t data_written_;
  uint64_t total_data_written_;

  uint64_t data_written_in_slice_;
  uint64_t size_of_slice_;

  double last_duration_;
  RTTimer timer_waiting_;
  RTTimer timer_other_;
  RTTimer timer_writing_;
};


#endif // INPUT_NODE_DATA_WRITER_H_INCLUDED
