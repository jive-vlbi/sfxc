/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - The definition of the Input_node_integer_delay_tasklet.
 */

#include <sched.h>
#include "input_node_integer_delay_tasklet.h"

Input_node_integer_delay_tasklet::Input_node_integer_delay_tasklet(){}
Input_node_integer_delay_tasklet::~Input_node_integer_delay_tasklet(){}

/*****************************************************************************
* @desc Empy all the queue.
*****************************************************************************/
void Input_node_integer_delay_tasklet::empty_input_queue()
{
  for (size_t i = 0; i < integer_delays_.size(); i++)
    {
      integer_delays_[i]->empty_input_queue();
    }
}


/*****************************************************************************
* @desc Stop the thread.
*****************************************************************************/
void Input_node_integer_delay_tasklet::stop()
{
  isrunning_=false;
}


/*****************************************************************************
* @desc The main computation loop that is executed in the data_writing thread
*****************************************************************************/
void Input_node_integer_delay_tasklet::do_execute()
{
  bool did_work=false;
  data_processed_=0;

  timer_.start();
  while ( isrunning_ ){
      did_work = false;
      for (size_t i=0; i<integer_delays_.size(); i++) {
          while (integer_delays_[i]->has_work()) {
              integer_delays_[i]->do_task();
              did_work = true;
            }
        }
      if ( !did_work )
        {
          //sched_yield();
          usleep(10000);
        }
    }
  timer_.stop();
}

/*****************************************************************************
* @desc Add a new channel.
*****************************************************************************/
void Input_node_integer_delay_tasklet::add_channel()
{
  integer_delays_.push_back( Integer_delay_correction_per_channel::new_sptr() );
}

/*****************************************************************************
* @desc Channel count
*****************************************************************************/
size_t Input_node_integer_delay_tasklet::number_channel()
{
  return integer_delays_.size();
}

/*****************************************************************************
* @desc Connect the input queue of a datawriter to an external output queue.
* @param int nr_stream The identifier of the stream.
* @param Input_node_types::Fft_buffer_ptr buffer is a queue containing FFTs
*****************************************************************************/
void
Input_node_integer_delay_tasklet::connect_to(int nr_stream,
    Input_node_types::Channel_buffer_ptr queue)
{
  SFXC_ASSERT( nr_stream < integer_delays_.size() );
  integer_delays_[nr_stream]->connect_to(queue);
}

/*****************************************************************************
* @desc Initialize the stream with the given parameters.
* @param int nr_stream The identifier of the stream.
* @param const Input_node_parameters &params
*****************************************************************************/
void Input_node_integer_delay_tasklet::set_parameters(int nr_stream,
    const Input_node_parameters &param,
    int nr_node)
{
  SFXC_ASSERT( nr_stream < integer_delays_.size() );
  integer_delays_[nr_stream]->set_parameters(param, nr_node);
}

/*****************************************************************************
* @desc Initialize with a delay table.
* @param Delay_table_akima &table the delay table
*****************************************************************************/
void Input_node_integer_delay_tasklet::set_delay_table(Delay_table_akima &table)
{
  for (size_t i=0; i<integer_delays_.size(); i++) {
      integer_delays_[i]->set_delay_table(table);
    }
}

/*****************************************************************************
 * @desc Add a new time interval to process. This information is queued in
 * the object for future processing.
 * @param uint64_t start_time The start time given in usec.
 * @param uint64_t stopt_time The start time given in usec.
 *****************************************************************************/
void
Input_node_integer_delay_tasklet::add_time_interval(uint64_t start_time_usec,
    uint64_t stop_time_usec)
{
  for (unsigned int i=0;i<integer_delays_.size(); i++){
      integer_delays_[i]->add_time_interval(start_time_usec, stop_time_usec);
    }
}

/*****************************************************************************
* @desc Get the output buffer associated with the channel nr_stream
*****************************************************************************/
Input_node_types::Fft_buffer_ptr
Input_node_integer_delay_tasklet::get_output_buffer(int nr_stream)
{
  SFXC_ASSERT( nr_stream < integer_delays_.size() );
  return integer_delays_[nr_stream]->get_output_buffer();
}

int Input_node_integer_delay_tasklet::bytes_of_output()
{
  SFXC_ASSERT( 0 < integer_delays_.size() );
  return integer_delays_[0]->bytes_of_output();
}
