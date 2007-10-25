/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Correlator_node.h"

#include "Data_reader_buffer.h"
#include "Data_writer_buffer.h"
#include "utils.h"

Correlator_node::Correlator_node(int rank, int nr_corr_node, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   integration_slice(get_log_writer()),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED),
   buffer_size(buff_size), 
   nr_corr_node(nr_corr_node),
   startIS(-1)
{
  get_log_writer()(1) << "Correlator_node(" << nr_corr_node << ")" << std::endl;
  
  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);

  
  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
  
  MPI_Send(&nr_corr_node, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, 
           MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
  assert(output_buffer.empty());
}

void Correlator_node::start()
{
  while (true) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        break;
      }
      case CORRELATING: {
        get_log_writer()(2) << " status = CORRELATING" << std::endl;
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_CORRELATING;
        }

        if (status==CORRELATING) {
          switch(correlate_state) {
            case INITIALISE_TIME_SLICE: 
            {
              if (data_readers_ctrl.get_data_reader(0)->data_counter()!=0) {
                DEBUG_MSG("data_readers_ctrl.get_data_reader(0)->data_counter()!=0");
              }
              assert(data_readers_ctrl.get_data_reader(0)->data_counter()==0);
              get_log_writer()(2) << " correlate_state = INITIALISE_TIME_SLICE" << std::endl;
              // Initialise the correlator for a new time slice:

              startIS=correlation_parameters.start_time;

              for (size_t sn=0; 
                   sn<data_readers_ctrl.number_of_data_readers(); sn++) {
                get_integration_slice().init_reader(sn,startIS);
              }
              correlate_state = CORRELATE_INTEGRATION_SLICE;
              break;
            }
            case CORRELATE_INTEGRATION_SLICE: 
            {
              get_log_writer()(2) << " correlate_state = CORRELATE_INTEGRATION_SLICE" << std::endl;
              // Do one integration step:
              //while still time slices and data are available
              if (startIS >= correlation_parameters.stop_time) {
                correlate_state = END_TIME_SLICE;
                break;
              }

              //process the next time slice:
              get_integration_slice().correlate();
              
              for (size_t i=0; i<correlation_parameters.station_streams.size(); i++) {
                int stream_nr = correlation_parameters.station_streams[i].station_stream;
                assert(stream_nr < data_readers_ctrl.number_of_data_readers());
              }

              //set start of next time slice to: start of time slice + time to average
              assert(correlation_parameters.integration_time > 0);
              startIS += correlation_parameters.integration_time; //in usec

              break;
            }
            case END_TIME_SLICE: 
            {
              get_log_writer()(2) << " correlate_state = END_TIME_SLICE" << std::endl;
              // Finish processing a time slice:
              for (size_t sn=0; 
                   sn<data_readers_ctrl.number_of_data_readers(); sn++) {
                int bytes_to_read =
                  data_readers_ctrl.get_data_reader(sn)->get_size_dataslice();
                int bytes_read = 0;
                // NGHK: TODO: get this from the sample_reader instead of directly from the data_reader
                while (bytes_to_read > 0) {
                  bytes_read =
                    data_readers_ctrl.get_data_reader(sn)->get_bytes(bytes_to_read-bytes_read,
                                                                     NULL);
                  assert(bytes_read > 0);
                  bytes_to_read -= bytes_read;
                }
                assert(data_readers_ctrl.get_data_reader(sn)->end_of_dataslice());
                data_readers_ctrl.get_data_reader(sn)->reset_data_counter();
              }

              // NGHK: Maybe a check on the data writer byte-size here??
              data_writer_ctrl.get_data_writer(0)->reset_data_counter();
              // Notify manager node:
              int32_t msg = get_correlate_node_number();
              MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                       MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, 
                       MPI_COMM_WORLD);

              status = STOPPED;
              correlate_state = INITIALISE_TIME_SLICE;

              break;
            }
          }
        }
        break;
      }
      case END_CORRELATING: {
        return;
      }
    }
  }
}

void Correlator_node::start_correlating(Correlation_parameters &param) {
  assert(status != CORRELATING); 
  
  correlation_parameters = param;
  get_integration_slice().set_parameters(correlation_parameters);

  int bytes = 
    ((int64_t)(correlation_parameters.stop_time-
               (correlation_parameters.start_time-MAX_DELAY)) *
    correlation_parameters.sample_rate *
    correlation_parameters.bits_per_sample) / 8000;

  
  for (size_t i=0; i<correlation_parameters.station_streams.size(); i++) {
    int stream_nr = correlation_parameters.station_streams[i].station_stream;
    assert(stream_nr < data_readers_ctrl.number_of_data_readers());
    
    data_readers_ctrl.get_data_reader(stream_nr)->set_size_dataslice(bytes);
  }

  for (int i=0; i<bits2float_converters.size(); i++) {
     if (bits2float_converters[i] != 
         boost::shared_ptr<Bits_to_float_converter>()) {
       bits2float_converters[i]->set_bits_per_sample(correlation_parameters.bits_per_sample);
     }
   }

   get_integration_slice().set_start_time(correlation_parameters.start_time);

   status=CORRELATING; 
   correlate_state = INITIALISE_TIME_SLICE; 
}

void Correlator_node::add_delay_table(int sn, Delay_table_akima &table) {
  get_integration_slice().set_delay_table(sn, table);
}


//void
//Correlator_node::
//set_parameters(Correlation_parameters &correlation_parameters_) {
//  // NGHK: No reference stations for now
//  correlation_parameters = correlation_parameters_;
//  get_integration_slice().set_parameters(correlation_parameters, -1, -1);
//}


void Correlator_node::hook_added_data_reader(size_t stream_nr) {
   // NGHK: TODO: Make sure a time slice fits
   boost::shared_ptr< Semaphore_buffer<input_value_type> > 
     buffer(new Semaphore_buffer<input_value_type>(25));
   data_readers_ctrl.set_buffer(stream_nr, buffer);
  
   boost::shared_ptr<Bits_to_float_converter> 
     sample_reader(new Bits_to_float_converter());
   sample_reader->set_data_reader(data_readers_ctrl.get_data_reader(stream_nr));
   
   if (bits2float_converters.size() <= stream_nr) {
     bits2float_converters.resize(stream_nr+1, 
                                  boost::shared_ptr<Bits_to_float_converter>());
   }
   bits2float_converters[stream_nr] = sample_reader;

   get_integration_slice().set_sample_reader(stream_nr,sample_reader);
}

void Correlator_node::hook_added_data_writer(size_t i) {
  assert(i == 0);

  get_integration_slice().set_data_writer(data_writer_ctrl.get_data_writer(i));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

/** Number of integration steps done in the current time slice **/
int Correlator_node::number_of_integration_steps_in_time_slice() {
  assert(false);
  return 0;
}

/** Size in bytes of the output of one integration step **/
int Correlator_node::output_size_of_one_integration_step() {
  assert(false);
  return 0;
}
