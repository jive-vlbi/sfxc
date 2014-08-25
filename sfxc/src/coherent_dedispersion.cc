#include "coherent_dedispersion.h"

Coherent_dedispersion::Coherent_dedispersion(int stream_nr_): 
      output_queue(Delay_queue_ptr(new Delay_queue())),
      stream_nr(stream_nr_), output_memory_pool(32),
      current_buffer(0){
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
  const int n_input_fft=input_data.size() / fft_size_dedispersion;
  total_input_fft += n_input_fft;
  if((RANK_OF_NODE == -10) && (current_fft > nffts_per_integration -1000)) 
  //if((RANK_OF_NODE == 10) && (stream_nr == 0)) 
    std::cerr<<"nfft = " <<n_input_fft 
             << ", size = " << input_data.size()
             << ", current_time " << (int64_t) current_time.get_time_usec()
             << ", current_fft = " << current_fft
             << ", input fft nr =" << total_input_fft
             << ", queue_length " << input_queue->size()
             << "\n";

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
  //Time ttt = Time(55870, 79405.0);
  Time ttt = Time(55870, 0.);
  if((RANK_OF_NODE == -10) && (current_fft > nffts_per_integration -1000) && (current_time >= ttt)) 
  //if((RANK_OF_NODE == 10) && (stream_nr == 0)) 
                         std::cerr << "fft = " << current_fft 
                                   << " / " << nffts_per_integration
                                   << ", outpos = " << out_pos
                                   << "\n";
  // Write output data
  if(out_pos > 0){
    cur_output->data.resize(out_pos);
    output_queue->push(cur_output);
    if((RANK_OF_NODE == -10) && (current_fft > nffts_per_integration -1000) && (current_time >= ttt)) 
    //if((RANK_OF_NODE == 10) && (stream_nr == 0)) 
                           std::cerr << "push, outpos = " << out_pos 
                                     << ", queue_size = " << output_queue->size()
                                     << "\n";
  }
}

void
Coherent_dedispersion::empty_output_queue(){
  while (output_queue->size() > 0)
    output_queue->pop();
}

void
Coherent_dedispersion::overlap_add(){
  if(RANK_OF_NODE == -10) std::cerr << "overlap_add, fft =" << current_fft
                                   << " / " << nffts_per_integration
                                   << "\n";
  Memory_pool_vector_element<FLOAT> &data = cur_output->data;
  //NB : fft_size_dedispersion >= fft_size_correlation
  const int step_size = fft_size_correlation / 2;
  const int nstep=fft_size_dedispersion/step_size;

  for(int n=0; (n < nstep) && (current_fft < nffts_per_integration); n++){
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
  if((current_fft < -nffts_per_integration) && (current_fft > 499988) && (input_queue->size() == 0)){
    std::cerr << RANK_OF_NODE << " : PROBLEM current_fft = " << current_fft 
              << ", time = " << current_time 
              << ", " << (int64_t) current_time.get_time_usec()<< "\n";
    problem = true;
  }else if((current_fft == nffts_per_integration) && problem){
    std::cerr << RANK_OF_NODE << " : NO PROBLEM current_fft = " << current_fft 
              << ", time = " << current_time 
              << ", " << (int64_t) current_time.get_time_usec()<< "\n";
  }
}

void
Coherent_dedispersion::create_dedispersion_filter(){
  filter.resize(fft_size_dedispersion+1);
  // Round channel offset to the nearest sample
  double fract_offset = channel_offset - 
                        integer_channel_offset.get_time_usec();
  double dnu = channel_bw / fft_size_dedispersion;
  double f0= channel_freq + sideband*channel_bw/2;
  double arg0 = 0;//channel_offset * channel_freq;
  for(int i=0; i<fft_size_dedispersion+1; i++){
    double f = sideband*dnu*(i-fft_size_dedispersion/2);
    double arg1=sideband*f*f*DM / (2.41e-10*f0*f0*(f+f0)); // For f, f0 in [MHz] 
    double arg2 = 0;// fract_offset*dnu*i; FIXME restore
    double arg = arg0+arg1+arg2;
    arg = -2*M_PI*(arg - floor(arg)); 
    filter[i] = std::complex<FLOAT>(cos(arg)/fft_size_dedispersion, 
                                    sin(arg)/fft_size_dedispersion);
    //filter[i] = std::complex<FLOAT>(cos(arg), 
    //                                sin(arg));
    std::cerr.precision(8);
    if((RANK_OF_NODE == 18) &&
       (current_fft == 0) &&
       (stream_nr == 0))
      std::cerr << "filter["<<i<<"]="<<filter[i] 
                << ", f= " << f << ", f0=" << f0
                << ", fract_offset = " << fract_offset
                << ", dnu = "<< dnu<<", DM="<< DM<< "\n";
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
Coherent_dedispersion::set_parameters(const Correlation_parameters &parameters, Pulsar &pulsar){
  total_input_fft = 0;
  empty_output_queue();
  problem = false;
  int stream_idx = 0;
  while ((stream_idx < parameters.station_streams.size()) &&
         (parameters.station_streams[stream_idx].station_stream != stream_nr))
    stream_idx++;
  if (stream_idx == parameters.station_streams.size()) {
    // Data stream is not participating in current time slice
    return;
  }

  DM = pulsar.polyco_params[0].DM;
  channel_freq = parameters.channel_freq/1000000.; // In MHz
  channel_bw = parameters.bandwidth / 1000000.; // In MHz
  sideband = (parameters.sideband == 'L') ? -1 : 1;
  fft_size_dedispersion = parameters.fft_size_dedispersion;
  fft_size_correlation = parameters.fft_size_correlation;
  sample_rate = parameters.sample_rate;
  channel_offset = parameters.channel_offset;
  int n = round(channel_offset * (sample_rate/1000000));
  integer_channel_offset = Time(0);
  integer_channel_offset.set_sample_rate(sample_rate);
  integer_channel_offset.inc_samples(n);
  start_time = parameters.integration_start;
  stop_time = parameters.integration_start + parameters.integration_time; 
  if(RANK_OF_NODE == 10){
  std::cout.precision(16);
  std::cout << RANK_OF_NODE << " :  integer_channel_offset = " <<  integer_channel_offset.get_time_usec()
            << ", channel_offset = " << channel_offset  << "\n";
  std::cout << RANK_OF_NODE << " : start_time = " << start_time 
            << ", " << start_time.get_time_usec() << "\n";
  std::cout << RANK_OF_NODE << " : stop_time = " << stop_time 
            << ", " << stop_time.get_time_usec() << "\n";}
  current_time = parameters.stream_start;
  current_time.set_sample_rate(sample_rate);
  
  current_fft = 0;

  nffts_per_integration =
      Control_parameters::nr_ffts_to_output_node(
        parameters.integration_time,
        parameters.sample_rate,
        parameters.fft_size_correlation);
  // Initialize buffers
  time_buffer[0].resize(2*fft_size_dedispersion);
  time_buffer[1].resize(2*fft_size_dedispersion);
  dedispersion_buffer.resize(fft_size_dedispersion+1);
  zeropad_buffer.resize(2*fft_size_dedispersion);
  // zero padding
  SFXC_ZERO_F(&zeropad_buffer[fft_size_dedispersion], fft_size_dedispersion); 

  // Initialize the FFT's
  fft.resize(2*fft_size_dedispersion);
  create_dedispersion_filter();
}
