/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *            Aard Keimpema <keimpema@jive.nl>, 2009
 *
 *  This file contains:
 *     - The definition of the Input_node_data_writer_tasklet.
 */
#include <sched.h>
#include "input_node_data_writer_tasklet.h"

Input_node_data_writer_tasklet::Input_node_data_writer_tasklet(){}
Input_node_data_writer_tasklet::~Input_node_data_writer_tasklet(){
  empty_input_queue();
  if(data_writer_thread_pool.still_running())
    data_writer_thread_pool.stop_all();

  data_writer_thread_pool.wait_for_all_termination();
}


/*****************************************************************************
* @desc Empy all the queue.
*****************************************************************************/
void Input_node_data_writer_tasklet::empty_input_queue()
{
    for (size_t i = 0; i < data_writers_.size(); i++)
    {
        data_writers_[i]->empty_input_queue();
    }
}


/*****************************************************************************
* @desc Stop all writer threads.
*****************************************************************************/
void Input_node_data_writer_tasklet::stop_threads()
{
  empty_input_queue();
  data_writer_thread_pool.stop_all();
}

/*****************************************************************************
* @desc Add a new channel.
*****************************************************************************/
void Input_node_data_writer_tasklet::add_channel()
{
    data_writers_.push_back( Input_node_data_writer::new_sptr() );
}

/*****************************************************************************
* @desc Channel count
*****************************************************************************/
size_t Input_node_data_writer_tasklet::number_channel()
{
    return data_writers_.size();
}


/*****************************************************************************
* @desc Connect the input queue of a datawriter to an external output queue.
* @param int nr_stream The identifier of the stream.
* @param Input_node_types::Fft_buffer_ptr buffer is a queue containing FFTs
*****************************************************************************/
void Input_node_data_writer_tasklet::connect_to(int nr_stream,
                                        Input_node_types::Channel_buffer_ptr buffer)
{
  SFXC_ASSERT( nr_stream < data_writers_.size() );
  data_writers_[nr_stream]->connect_to(buffer);
  data_writer_thread_pool.register_thread(data_writers_[nr_stream]->start());
}

/*****************************************************************************
* @desc Initialize the stream with the given parameters.
* @param int nr_stream The identifier of the stream.
* @param const Input_node_parameters &params
*****************************************************************************/
void Input_node_data_writer_tasklet::set_parameters(int nr_stream,
                                            const Input_node_parameters &params)
{
  SFXC_ASSERT( nr_stream < data_writers_.size() );
  data_writers_[nr_stream]->set_parameters(params);
}


/*****************************************************************************
* @desc Add a pair of (data_writer, size_slice) to a given channel-queue.
* @param int nr_stream The identifier of the stream.
* @param Data_writer_sptr wr The writer on which to stream the data
* @param int size the amount of byte to send to the given writer
*****************************************************************************/
void Input_node_data_writer_tasklet::add_timeslice_to_stream(int nr_stream,
                                                     Data_writer_sptr wr,
                                                     int64_t size)
{
  SFXC_ASSERT( nr_stream < data_writers_.size() );
  data_writers_[nr_stream]->add_timeslice(wr, size);
}

/*****************************************************************************
 * @desc Add a new delay table to the writer, the content is to be send to
 * the correlator node.
 * @param delay only contains the delay at time positions where the integer delay changes
 *****************************************************************************/
void
Input_node_data_writer_tasklet::add_delay(Delay_memory_pool_element delay)
{
    for (size_t i=0; i<data_writers_.size(); i++)
            data_writers_[i]->add_delay(delay);
}

/*****************************************************************************
 * @desc Add a new time interval to the writer
 * @param start Start time of interval
 * @param stop Stop time of interval
 *****************************************************************************/
void
Input_node_data_writer_tasklet::add_time_interval(Time start, Time stop) {
  for (size_t i=0; i<data_writers_.size(); i++)
    data_writers_[i]->add_time_interval(start, stop);
}

/*****************************************************************************
 * @desc Return the earliest current_time from the data writers
 *****************************************************************************/
Time
Input_node_data_writer_tasklet::get_current_time(){
  int i=-1;
  int size=data_writers_.size();
  Time time;
  // First find a valid time
   do{
     i++;
     time = data_writers_[i]->get_current_time();
   }while((i < size-1) && (time == Time()));

   for (int j = 1; j < size; j++){
    Time new_time = data_writers_[j]->get_current_time();
    if(new_time < time)
     time = new_time;
   }

  return time;
}
