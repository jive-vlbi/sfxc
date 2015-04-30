#include "coherent_dedispersion.h"

Coherent_dedispersion::Coherent_dedispersion(int stream_nr_,
                         SFXC_FFT  &fft_, 
                         Memory_pool_vector_element<std::complex<FLOAT> > &filter_,
                         Memory_pool_vector_element<std::complex<FLOAT> > &dedispersion_buffer_,
                         Memory_pool_vector_element<FLOAT> &zeropad_buffer_):
      output_queue(Delay_queue_ptr(new Delay_queue())),
      stream_nr(stream_nr_), output_memory_pool(2, NO_RESIZE), 
      current_buffer(0), fft(fft_), filter(filter_), 
      dedispersion_buffer(dedispersion_buffer_),
      zeropad_buffer(zeropad_buffer_) {
}

Coherent_dedispersion::~Coherent_dedispersion(){
  // Clear variable in order not to confuse the reference counting
  cur_output = Delay_queue_element();
  while(output_queue->size() > 0)
    output_queue->pop();
}

void
Coherent_dedispersion::do_task(){
  Delay_queue_element input = input_queue->front_and_pop();
  Memory_pool_vector_element<FLOAT> &input_data = input->data;
  if(current_time >= stop_time)
    return;
  const int n_input_fft=input_data.size() / fft_size_dedispersion;
  total_input_fft += n_input_fft;
/*  if((RANK_OF_NODE == 5) && (current_fft > nffts_per_integration -1000)) 
    std::cerr<<"nfft = " <<n_input_fft 
             << ", size = " << input_data.size()
             << ", current_time " << (int64_t) current_time.get_time_usec()
             << ", current_fft = " << current_fft
             << ", input fft nr =" << total_input_fft
             << ", queue_length " << input_queue->size()
             << "\n";*/

  // Allocate output buffer
  allocate_element(n_input_fft*fft_size_dedispersion/fft_size_correlation);

  for(int i=0; i<n_input_fft; i++){
    // Apply dedispersion
    memcpy(&zeropad_buffer[0], 
           &input_data[i*fft_size_dedispersion], 
           fft_size_dedispersion*sizeof(FLOAT));
    fft.rfft(&zeropad_buffer[0], &dedispersion_buffer[0]);
    SFXC_MUL_FC(&dedispersion_buffer[0], &filter[0], 
                &dedispersion_buffer[0], fft_size_dedispersion + 1);
    fft.irfft(&dedispersion_buffer[0], &time_buffer[current_buffer][0]);

    // Perform overlap-add
    overlap_add();
    current_buffer = 1 - current_buffer;
    current_time.inc_samples(fft_size_dedispersion);
  }
  if((RANK_OF_NODE == -10) && (stream_nr == 0)) 
    std::cerr<<"now = " <<current_time << ", start = " << start_time
             <<", stop_time = " << stop_time << "\n";
  // Write output data
  if(out_pos > 0){
    cur_output->data.resize(out_pos);
    output_queue->push(cur_output);
  }
}

void
Coherent_dedispersion::empty_output_queue(){
  while (output_queue->size() > 0)
    output_queue->pop();
}

void
Coherent_dedispersion::overlap_add(){
  Memory_pool_vector_element<FLOAT> &data = cur_output->data;
  //NB : fft_size_dedispersion >= fft_size_correlation
  const int step_size = fft_size_correlation / 2;
  int nstep=fft_size_dedispersion/step_size;
  // First time only fft_size_dedispersion/2 of data is send
  if(current_fft==0) 
    nstep /= 2;

  for(int n=0; (n < nstep); n++){
    int i, j;
    if(n<nstep/2){
      i = 3*fft_size_dedispersion/2 + n*step_size;
      j = fft_size_dedispersion/2 + n*step_size;
    }else{
      i = (n-nstep/2)*step_size;
      j = fft_size_dedispersion + (n-nstep/2) *step_size;
    }
    if ((RANK_OF_NODE == -10) && (stream_nr == 0))
      std::cerr << "n="<<n << "/"<<nstep<<", i="<<i<<", j="<<j
                << ", fft_dedisp="<< fft_size_dedispersion<<", fft_corr="<<fft_size_correlation
                << ", outpos=" << out_pos<<"\n";
    // Sum overlapping windows
    SFXC_ADD_F(&time_buffer[current_buffer][i],
               &time_buffer[1-current_buffer][j],
               &data[out_pos],
               step_size);
    out_pos += step_size;
    if(out_pos % fft_size_correlation == 0)
      current_fft += 1;
  }
}

void
Coherent_dedispersion::allocate_element(int nfft){
  cur_output = output_memory_pool.allocate();
  cur_output->data.resize(fft_size_correlation*nfft);
  out_pos = 0;
}

bool
Coherent_dedispersion::has_work(){
  if (input_queue->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  return true;
}

void Coherent_dedispersion::connect_to(Delay_queue_ptr buffer) {
  input_queue = buffer;
}

Coherent_dedispersion::Delay_queue_ptr
Coherent_dedispersion::get_output_buffer() {
  SFXC_ASSERT(output_queue != Delay_queue_ptr());
  return output_queue;
}

void 
Coherent_dedispersion::set_parameters(const Correlation_parameters &parameters)
{
  total_input_fft = 0;
  empty_output_queue();
  int stream_idx = 0;
  while ((stream_idx < parameters.station_streams.size()) &&
         (parameters.station_streams[stream_idx].station_stream != stream_nr))
    stream_idx++;
  if (stream_idx == parameters.station_streams.size()) {
    // Data stream is not participating in current time slice
    return;
  }

  fft_size_dedispersion = parameters.fft_size_dedispersion;
  fft_size_correlation = parameters.fft_size_correlation;
  sample_rate = parameters.sample_rate;
  start_time = parameters.integration_start;
  stop_time = parameters.integration_start + parameters.integration_time; 
  if(RANK_OF_NODE == 10){
    std::cout.precision(16);
    std::cout << RANK_OF_NODE << " : start_time(" << stream_nr << ") = " << start_time 
              << ", " << start_time.get_time_usec() << "\n";
    std::cout << RANK_OF_NODE << " : stop_time(" << stream_nr << ") = " << stop_time 
              << ", " << stop_time.get_time_usec() << "\n";
  }
  current_time = parameters.stream_start;
  current_time.set_sample_rate(sample_rate);
  current_time.inc_samples(-fft_size_dedispersion/2);
  
  current_fft = 0;

  // Initialize buffers
  time_buffer[0].resize(2*fft_size_dedispersion);
  time_buffer[1].resize(2*fft_size_dedispersion);
}
