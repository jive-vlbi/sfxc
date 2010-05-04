/* Copyright (c) 2009 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s):  Aard Keimpema <keimpema@jive.nl>, 2009
 *
 */

#ifndef CORRELATOR_NODE_BIT2FLOAT_TASKLET_H
#define CORRELATOR_NODE_BIT2FLOAT_TASKLET_H

#include "utils.h"
#include "timer.h"
#include "thread.h"
#include "correlator_node_types.h"
#include "control_parameters.h"
#include "bit2float_worker.h"

class Correlator_node_bit2float_tasklet : public Thread {
public:
  typedef Correlator_node_types::Channel_circular_input_buffer      Channel_circular_input_buffer;
  typedef Correlator_node_types::Channel_circular_input_buffer_ptr  Channel_circular_input_buffer_ptr;

  Correlator_node_bit2float_tasklet();
  virtual ~Correlator_node_bit2float_tasklet();
  /*****************************************************************************
  * @desc Main function called in the thread. This function is inherited from
  * the Thread class.
  *****************************************************************************/
  void do_execute();

  /*****************************************************************************
  * @desc Request to stop the current thread.
  *****************************************************************************/
  void stop();

  /*****************************************************************************
  * @desc Channel count
  *****************************************************************************/
  size_t number_channel();

  /*****************************************************************************
  * @desc Connect the input queue of the specified channel to a new queue.
  * @param int nr_stream The identifier of the stream.
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void connect_to(int nr_stream, bit_statistics_ptr statistics, Channel_circular_input_buffer_ptr);

	/*****************************************************************************
  * @desc Retreive the output queue for the specified channel.
  * @param int nr_stream The identifier of the stream.
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  Bit2float_worker::Output_queue_ptr get_output_buffer(int nr_stream);

  /*****************************************************************************
  * @desc Initialize the stream with the given parameters.
  * @param const Correlator_node_parameters &params
  *****************************************************************************/
  void set_parameters(const Correlation_parameters &params);

  /*****************************************************************************
  * @desc Clear all input links
  *****************************************************************************/
  void empty_input_queue();

	/*****************************************************************************
  * @desc Return the amount of data processed by this tasklet.
  *****************************************************************************/
 	inline uint64_t get_num_processed_bytes(){ return data_processed_; }

  /*****************************************************************************
  * @desc Return the integer delay array.
  *****************************************************************************/
  std::vector< Bit2float_worker_sptr >& bit2float_workers();

private:
  std::vector<Bit2float_worker_sptr>    bit2float_workers_;

  /// Amount of processing time.
  Timer timer_;

  uint64_t data_processed_;
};

#endif
