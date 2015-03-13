#include <sched.h>
#include "correlator_node_bit2float_tasklet.h"
#include "bit_statistics.h"

#define MINIMUM_PROCESSED_SAMPLES 1024

Correlator_node_bit2float_tasklet::Correlator_node_bit2float_tasklet() {}

Correlator_node_bit2float_tasklet::~Correlator_node_bit2float_tasklet() {}

void Correlator_node_bit2float_tasklet::empty_input_queue(){
  for (size_t i = 0; i < bit2float_workers_.size(); i++){
      bit2float_workers_[i]->empty_input_queue();
  }
}

void Correlator_node_bit2float_tasklet::stop(){
  isrunning_=false;
}

void Correlator_node_bit2float_tasklet::do_execute(){
  int processed_samples;
  data_processed_=0;

  timer_.start();
  while ( isrunning_ ){
    processed_samples=0;
    for (size_t i=0; i<bit2float_workers_.size(); i++) {
      if (bit2float_workers_[i]->has_work()) {
        processed_samples += bit2float_workers_[i]->do_task();
      }
    }
    if ( processed_samples < MINIMUM_PROCESSED_SAMPLES ){
      //sched_yield();
      usleep(1000);
    }
  }
  timer_.stop();
}
size_t Correlator_node_bit2float_tasklet::number_channel(){
  return bit2float_workers_.size();
}

void
Correlator_node_bit2float_tasklet::connect_to(int nr_stream, bit_statistics_ptr statistics,
    Channel_circular_input_buffer_ptr buffer)
{
  if (bit2float_workers_.size() <= nr_stream) {
    bit2float_workers_.resize(nr_stream+1, boost::shared_ptr<Bit2float_worker>());
  }
  bit2float_workers_[nr_stream] = Bit2float_worker::new_sptr(nr_stream, statistics);
  SFXC_ASSERT( nr_stream < bit2float_workers_.size() );
  bit2float_workers_[nr_stream]->connect_to(buffer);
}

void 
Correlator_node_bit2float_tasklet::set_parameters(const Correlation_parameters &param,
                                                  std::vector<Delay_table_akima> &delays){
  for(int i=0; i<bit2float_workers_.size(); i++)
    bit2float_workers_[i]->set_new_parameters(param, delays[i]);
}

Bit2float_worker::Output_queue_ptr
Correlator_node_bit2float_tasklet::get_output_buffer(int nr_stream){
  SFXC_ASSERT( nr_stream < bit2float_workers_.size() );
  return bit2float_workers_[nr_stream]->get_output_buffer();
}

std::vector<Bit2float_worker::Invalid> *
Correlator_node_bit2float_tasklet::get_invalid(int nr_stream){
  SFXC_ASSERT( nr_stream < bit2float_workers_.size() );
  return bit2float_workers_[nr_stream]->get_invalid();
}
 
std::vector< Bit2float_worker_sptr >& 
Correlator_node_bit2float_tasklet::bit2float_workers() {
  return bit2float_workers_;
}
