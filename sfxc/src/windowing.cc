#include "windowing.h"

Windowing::Windowing(int stream_nr_): 
      output_queue(Correlation_queue_ptr(new Correlation_queue())),
      stream_nr(stream_nr_), output_memory_pool(32){
}

Windowing::~Windowing(){
  // Clear variable in order not to confuse the reference counting
  if(output_queue != Correlation_queue_ptr()){
    while(output_queue->size() > 0)
      output_queue->pop();
  }
}

void
Windowing::do_task(){
  Delay_queue_element input = input_queue->front_and_pop();
  Memory_pool_vector_element<FLOAT> &input_data = input->data;
  const int nfft = std::min(nffts_per_integration - current_fft,
                            (int)input_data.size() / fft_size_correlation);
  // Allocate new output buffer
  Correlation_queue_element cur_output = output_memory_pool.allocate();
  cur_output->stride = output_stride;
  SFXC_ASSERT(nfft > 0)
  cur_output->data.resize(output_stride*nfft);
  int out_fft = 0;

  int i = 0;
  while(i < input_data.size()){
    const int fft_step = fft_rot_size / 2;
    if(window_func == SFXC_WINDOW_NONE){
      memcpy(&buffers[buf][0], &input_data[i], fft_step*sizeof(FLOAT));
    }else{
      SFXC_MUL_F(&input_data[i], 
                 &window[fft_step], 
                 &buffers[buf][fft_step],
                 fft_step);
      SFXC_MUL_F(&input_data[i], 
                 &window[0], 
                 &buffers[1-buf][0], 
                 fft_step);
    }

    // Do the final fft from time to frequency
    if((current_time >= integration_start) && (out_fft < nfft)){
      fft_t2f.rfft(&buffers[buf][0], &temp_fft_buffer[0]);
      memcpy(&cur_output->data[out_fft * output_stride], 
             &temp_fft_buffer[temp_fft_offset], 
             (fft_size_correlation+1) * sizeof(std::complex<FLOAT>));
      out_fft += 1;
    }
    buf = 1-buf;
    i += fft_step;
    current_time.inc_samples(fft_step);
  }
  if (RANK_OF_NODE == -5)
    std::cerr << "nffts_per_integration = " << nffts_per_integration
              << ", current_fft = " << current_fft << "\n"; 
  if(out_fft > 0){
    current_fft += out_fft;
    if (out_fft != nfft)
      cur_output->data.resize(out_fft*output_stride);
    output_queue->push(cur_output);
  }
}

bool
Windowing::has_work(){
  if (input_queue->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  return true;
}

void Windowing::connect_to(Delay_queue_ptr buffer) {
  input_queue = buffer;
}

Windowing::Correlation_queue_ptr
Windowing::get_output_buffer() {
  SFXC_ASSERT(output_queue != Correlation_queue_ptr());
  return output_queue;
}

void 
Windowing::set_parameters(const Correlation_parameters &parameters){
  int stream_idx = 0;
  while ((stream_idx < parameters.station_streams.size()) &&
         (parameters.station_streams[stream_idx].station_stream != stream_nr))
    stream_idx++;
  if (stream_idx == parameters.station_streams.size()) {
    // Data stream is not participating in current time slice
    return;
  }

  fft_size_correlation = parameters.fft_size_correlation;
  fft_rot_size = (parameters.station_streams[stream_idx].sample_rate / parameters.sample_rate) * 2 * fft_size_correlation;
  window_func = parameters.window;
  integration_start = parameters.integration_start;
  current_time = parameters.stream_start;
  current_time.set_sample_rate(parameters.station_streams[stream_idx].sample_rate);
  if(window_func != SFXC_WINDOW_NONE)
    current_time.inc_samples(-fft_rot_size / 2);
  if(RANK_OF_NODE == 5){
  std::cout.precision(16);
  std::cout << RANK_OF_NODE << " : current_time = " << current_time
            << ", " << current_time.get_time_usec() << "\n";
  std::cout << RANK_OF_NODE << " : start_time = " << integration_start 
            << ", " << integration_start.get_time_usec() << "\n";
  std::cout << RANK_OF_NODE << " : stream_start = " << parameters.stream_start
            << ", " << parameters.stream_start.get_time_usec() << "\n";}
  
  current_fft = 0;

  nffts_per_integration =
      Control_parameters::nr_ffts_to_output_node(
        parameters.integration_time,
        parameters.sample_rate,
        parameters.fft_size_correlation);
  if (window_func != SFXC_WINDOW_NONE)
    nffts_per_integration -= 1;
  output_stride = parameters.fft_size_correlation + 4; // ensure 8 bytes allignment
  // Calculate the offset into temp_fft_buffer where we can find the
  // spectral points that we want to correlate.
  int64_t delta; 
  int64_t freq = parameters.station_streams[stream_idx].channel_freq;
  if (parameters.sideband != parameters.station_streams[stream_idx].sideband){
    int sb = (parameters.station_streams[stream_idx].sideband == 'L') ? -1 : 1;
    freq += sb * parameters.station_streams[stream_idx].bandwidth;
  }
  if (parameters.sideband == 'L')
    delta = freq - parameters.channel_freq;
  else
    delta = parameters.channel_freq - freq;
  SFXC_ASSERT(delta >= 0);
  // TODO Is this correct?
  temp_fft_offset = delta * 2 * fft_size_correlation / parameters.sample_rate;
  temp_fft_buffer.resize(fft_rot_size/2 + 4);
  // Initialize buffers
  buffers[0].resize(fft_rot_size);
  buffers[1].resize(fft_rot_size);
  buf = 0;
  // zero padding
  if(window_func == SFXC_WINDOW_NONE){
    memset(&buffers[0][fft_rot_size/2], 0, fft_rot_size/2*sizeof(FLOAT));
    memset(&buffers[1][fft_rot_size/2], 0, fft_rot_size/2*sizeof(FLOAT));
  }

  // Initialize the FFT's
  fft_t2f.resize(fft_rot_size);
  // FIXME we should check if this needs recomputing
  create_window();
}

void 
Windowing::create_window(){
  const int n = fft_rot_size;
  window.resize(n);
  switch(window_func){
  case SFXC_WINDOW_NONE:
    //  Identical to the case without windowing
    for (int i=0; i<n/2; i++)
      window[i] = 1;
    for (int i = n/2; i < n; i++)
      window[i] = 0;
    break;
  case SFXC_WINDOW_RECT:
    // rectangular window (including zero padding)
    for (int i=0; i<n/4; i++)
      window[i] = 0;
    for (int i = n/4; i < 3*n/4; i++)
      window[i] = 1;
    for (int i = 3*n/4 ; i < n ; i++)
      window[i] = 0;
    break;
  case SFXC_WINDOW_COS:
    // Cosine window
    for (int i=0; i<n; i++){
      window[i] = sin(M_PI * i /(n-1));
    }
    break;
  case SFXC_WINDOW_HAMMING:
    // Hamming window
    for (int i=0; i<n; i++){
      window[i] = 0.54 - 0.46 * cos(2*M_PI*i/(n-1));
    }
    break;
  case SFXC_WINDOW_HANN:
    // Hann window
    for (int i=0; i<n; i++){
      window[i] = 0.5 * (1 - cos(2*M_PI*i/(n-1)));
    }
    break;
  default:
    sfxc_abort("Invalid windowing function");
  }
}
