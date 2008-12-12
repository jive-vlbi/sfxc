/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - The declaration of a threaded object that apply integer delay
 *       to the input streams streams.
 *
 */
#ifndef INPUT_NODE_INTEGER_DELAY_TASKLET_H_INCLUDED
#define INPUT_NODE_INTEGER_DELAY_TASKLET_H_INCLUDED

#include "utils.h"
#include "timer.h"
#include "thread.h"
#include "input_node_types.h"
#include "control_parameters.h"
#include "integer_delay_correction_per_channel.h"

/*******************************************************************************
 * @author Damien Marchal
 * @class Input_node_integer_delay_tasklet
 * @desc  Get from a list of input queues the data to stream to the different
 *        correlation cores.
 * This class is far from perfect but as transitional step is it rather usefull
 * The data writing is then threaded. An improved version should probably
 * maintain a dependency tracker and use multiple thread to stream to different
 * location in parallel.
 ******************************************************************************/
class Input_node_integer_delay_tasklet : public Thread {

public:
  /** Ctor **/
  Input_node_integer_delay_tasklet();

  /** Dtor **/
  virtual ~Input_node_integer_delay_tasklet();

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
  * @desc Add a new channel.
  *****************************************************************************/
  void add_channel();

  /*****************************************************************************
  * @desc Channel count
  *****************************************************************************/
  size_t number_channel();

  /*****************************************************************************
  * @desc Connect the input queue of the specified channel to a new queue.
  * @param int nr_stream The identifier of the stream.
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void connect_to(int nr_stream, Input_node_types::Channel_buffer_ptr);

	/*****************************************************************************
  * @desc Retreive the output queue for the specified channel.
  * @param int nr_stream The identifier of the stream.
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  Input_node_types::Fft_buffer_ptr get_output_buffer(int nr_stream);

  /*****************************************************************************
  * @desc Initialize the stream with the given parameters.
  * @param int nr_stream The identifier of the stream.
  * @param const Input_node_parameters &params
  * assert( nr_stream < number_channels() )
  *****************************************************************************/
  void set_parameters(int nr_stream, const Input_node_parameters &params, int n);

  /*****************************************************************************
  * @desc Deplate the input_queues of all of the data_writers.
  *****************************************************************************/
  void empty_input_queue();

	/*****************************************************************************
  * @desc Add a new time interval to process. This information is queued in
  * the object for future processing.
  * @param uint64_t start_time THe start time given in usec.
  * @param uint64_t stopt_time THe start time given in usec.
  *****************************************************************************/
  void add_time_interval(uint64_t start_time, uint64_t stop_time);

	/*****************************************************************************
	* @desc Initialize with a delay table.
	* @param Delay_table_akima &table the delay table
	*****************************************************************************/
	void set_delay_table(Delay_table_akima &table);

	/*****************************************************************************
	* @desc Return the amount of bytes in the current timeslice.
	*****************************************************************************/
	int bytes_of_output();

	/*****************************************************************************
  * @desc Return the amount of data processed by this tasklet.
  *****************************************************************************/
 	inline uint64_t get_num_processed_bytes(){ return data_processed_; }

 	/*****************************************************************************
  * @desc Return the number of seconds since the start() is called.
  *****************************************************************************/
 	inline double get_sec(){ return timer_.measured_time(); }

  /*****************************************************************************
  * @desc Get the current time position of the delay modules
  *****************************************************************************/
  int64_t get_current_time();

private:
  /// Writers that will stream the data.
  std::vector<Integer_delay_correction_per_channel_sptr>    integer_delays_;

  /// Amount of processing time.
  Timer timer_;

  uint64_t data_processed_;
};

#endif // INPUT_NODE_INTEGER_DELAY_H_INCLUDED
