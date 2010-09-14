/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - The declaration of a threaded object that streams the data to a set
 *       of writers.
 */
#ifndef INPUT_NODE_DATA_WRITER_TASKLET_H_INCLUDED
#define INPUT_NODE_DATA_WRITER_TASKLET_H_INCLUDED

#include "utils.h"
#include "timer.h"
#include "input_node_types.h"
#include "control_parameters.h"
#include "input_node_data_writer.h"

/*******************************************************************************
 * @author Damien Marchal
 * @class Input_node_data_writer
 * @desc  Get from a list of input queues the data to stream to the different
 *        correlation cores.
 * This class is far from perfect but as transitional step is it rather usefull
 * The data writing is then threaded. An improved version should probably
 * maintain a dependency tracker and use multiple thread to stream to different
 * location in parallel.
 ******************************************************************************/
class Input_node_data_writer_tasklet {
public:
  /** Ctor **/
  Input_node_data_writer_tasklet();

  /** Dtor **/
  virtual ~Input_node_data_writer_tasklet();

  /*****************************************************************************
  * @desc Stop all data writer threads
  *****************************************************************************/
  void stop_threads();

  /*****************************************************************************
  * @desc Add a new channel.
  *****************************************************************************/
  void add_channel();

  /*****************************************************************************
  * @desc Channel count
  *****************************************************************************/
  size_t number_channel();

  /*****************************************************************************
  * @desc Connect the input queue of a datawriter to an external output queue.
  * @param int nr_stream The identifier of the stream.
  * @param Input_node_types::Fft_buffer_ptr buffer is a queue containing FFTs
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void connect_to(int nr_stream, Input_node_types::Channel_buffer_ptr buffer);

  /*****************************************************************************
  * @desc Initialize the stream with the given parameters.
  * @param int nr_stream The identifier of the stream.
  * @param const Input_node_parameters &params
   * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void set_parameters(int nr_stream, const Input_node_parameters &params);

  /*****************************************************************************
  * @desc Add a pair of (data_writer, size_slice) to a given channel-queue.
  * @param int nr_stream The identifier of the stream.
  * @param Data_writer_sptr wr The writer on which to stream the data
  * @param int64_t size the amount of samples to send to the given writer
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void add_timeslice_to_stream(int nr_stream, Data_writer_sptr wr, int64_t size);

  /*****************************************************************************
  * @desc Deplate the input_queues of all of the data_writers.
  *****************************************************************************/
  void empty_input_queue();

  /*****************************************************************************
  * @desc Return the amount of data processed by this tasklet.
  *****************************************************************************/
 	inline uint64_t get_num_processed_bytes(){ return data_processed_; }

	/*****************************************************************************
  * @desc Return the number of seconds since the start() is called.
  *****************************************************************************/
 	inline double get_sec(){ return timer_.measured_time(); }
  /*****************************************************************************
  * @desc Add a new delay table to the writer, the content is to be send to
  * the correlator node.
  * @param delay only contains the delay at time positions where the integer delay changes
  *****************************************************************************/
  void add_delay(Delay_memory_pool_element delay);

  /*****************************************************************************
  *  @desc Add a new time interval to the writer
  * @param start Start time of interval
  * @param sop Stop time of interval
  *****************************************************************************/
  void add_time_interval(Time start, Time stop);

  /*****************************************************************************
  * @desc Return the earliest current_time from the data writers
  *****************************************************************************/
  Time get_current_time();

private:
  /// Writers that will stream the data.
  std::vector<Input_node_data_writer_sptr>    data_writers_;
  ThreadPool data_writer_thread_pool;

  /// Amount of processing time.
  Timer timer_;

  uint64_t data_processed_;
};

#endif // INPUT_NODE_DATA_WRITER_TASKLET_H_INCLUDED
