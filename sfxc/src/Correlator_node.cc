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
#include "InData.h"

Correlator_node::Correlator_node(int rank, int nr_corr_node, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   integration_slice(get_log_writer()),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED),
   buffer_size(buff_size), nr_corr_node(nr_corr_node),
   startIS(-1)
{
  get_log_writer() << "Correlator_node()" << std::endl;;
  
  // set the log writer for ProcessData:
  ::set_log_writer(get_log_writer());

  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);
  
  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
  
  int i=0;
  MPI_Send(&i, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
  assert(output_buffer.empty());
}

void Correlator_node::start()
{
  while (true) {
    switch (status) {
      case END_CORRELATING: {
        return;
      }
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
                
                startIS=GenPrms.get_usStart();
  
                for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
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
              if (startIS >= GenPrms.get_usStart() + GenPrms.get_usDur()) {
                correlate_state = END_TIME_SLICE;
                break;
              }
                               
              //process the next time slice:
              get_integration_slice().correlate();
              //set start of next time slice to: start of time slice + time to average
              startIS += GenPrms.get_usTime2Avg(); //in usec
              
              
              break;
            }
          case END_TIME_SLICE: 
            {
              get_log_writer()(2) << " correlate_state = END_TIME_SLICE" << std::endl;
              
              // Finish processing a time slice:
              status = STOPPED;

              for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
                int bytes_to_read =
                  data_readers_ctrl.get_data_reader(sn)->get_size_dataslice();
                int bytes_read = 0;
                while (bytes_to_read != bytes_read) {
                  int status=data_readers_ctrl.get_data_reader(sn)->get_bytes(
                    bytes_to_read-bytes_read,
                    NULL);
                 assert(status > 0);
                 bytes_read+=status;
                }
                assert(
                  data_readers_ctrl.get_data_reader(sn)->end_of_dataslice());
                data_readers_ctrl.get_data_reader(sn)->reset_data_counter();
              }
  
              // NGHK: Maybe a check on the data writer byte-size here??
              get_data_writer().reset_data_counter();
              // Notify manager node:
              int32_t msg = 0;
              MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                       MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, 
                       MPI_COMM_WORLD);
              break;
            }
          }
        }
      }
    }
  }
}

void Correlator_node::start_correlating(int64_t us_start, int64_t duration) {
  assert(status != CORRELATING); 
  
  GenPrms.set_usStart(us_start);
  GenPrms.set_duration(duration);

  // 2*bwfl is the Nyquist sample rate
  // two bits sampling, hence 4 samples per byte
  int bits_per_sample = 2;
  int NYQUIST = 2;
  int bytes = (int)((BufTime*4 + duration*1000000)* 
                    (GenPrms.get_bwfl()*NYQUIST*bits_per_sample)/8/1000000);
  for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
    data_readers_ctrl.get_data_reader(sn)->set_size_dataslice(bytes);
  }

  get_integration_slice().set_start_time(us_start);

  status=CORRELATING; 
  correlate_state = INITIALISE_TIME_SLICE; 
}

void Correlator_node::add_delay_table(int sn, Delay_table_akima &table) {
  get_integration_slice().set_delay_table(sn, table);
}


void Correlator_node::set_parameters(RunP &runPrms, GenP &genPrms, StaP *staPrms) {
  get_integration_slice().set_parameters(genPrms, staPrms, 
					 runPrms.get_ref_station(0),
					 runPrms.get_ref_station(1));
}


void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  // NGHK: TODO: Make sure a time slice fits
  boost::shared_ptr< Semaphore_buffer<input_value_type> > 
    buffer(new Semaphore_buffer<input_value_type>(25));
  data_readers_ctrl.set_buffer(stream_nr, buffer);
  
  boost::shared_ptr<Bits_to_float_converter> 
    sample_reader(new Bits_to_float_converter());
  sample_reader->set_bits_per_sample(StaPrms->get_bps());
  sample_reader->set_data_reader(data_readers_ctrl.get_data_reader(stream_nr));
  
  get_integration_slice().set_sample_reader(stream_nr,sample_reader);
}

void Correlator_node::hook_added_data_writer(size_t i) {
  assert(i == 0);

  get_integration_slice().set_data_writer(data_writer_ctrl.get_data_writer(i));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::set_slice_number(int sliceNr_) {
  sliceNr = sliceNr_;
}

/** Number of integration steps done in the current time slice **/
int Correlator_node::number_of_integration_steps_in_time_slice() {
  return (int32_t) (GenPrms.get_duration()/GenPrms.get_time2avg());
}

/** Size in bytes of the output of one integration step **/
int Correlator_node::output_size_of_one_integration_step() {
  return sizeof(fftw_complex)*((GenPrms.get_n2fft()*GenPrms.get_pad())/2+1);
}
