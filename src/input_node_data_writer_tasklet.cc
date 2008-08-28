/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - The definition of the Input_node_data_writer_tasklet.
 */
#include <sched.h>
#include "input_node_data_writer_tasklet.h"

Input_node_data_writer_tasklet::Input_node_data_writer_tasklet(){}
Input_node_data_writer_tasklet::~Input_node_data_writer_tasklet(){}


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
* @desc Stop the thread.
*****************************************************************************/
void Input_node_data_writer_tasklet::stop()
{
  isrunning_=false;
}


/*****************************************************************************
* @desc The main computation loop that is executed in the data_writing thread
*****************************************************************************/
void Input_node_data_writer_tasklet::do_execute()
{
  bool did_work=false;

  data_processed_ = 0;

	timer_.start();
  while( isrunning_ ){
    did_work = false;
    for (size_t i=0; i<data_writers_.size(); i++)
    {
        while (data_writers_[i]->has_work())
        {
            data_processed_+=data_writers_[i]->do_task();
            did_work=true;
        }
    }
    if( !did_work )
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
                                        Input_node_types::Fft_buffer_ptr buffer)
{
  SFXC_ASSERT( nr_stream < data_writers_.size() );
  data_writers_[nr_stream]->connect_to(buffer);
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
                                                     int size)
{
  SFXC_ASSERT( nr_stream < data_writers_.size() );
  data_writers_[nr_stream]->add_timeslice(wr, size);
}

