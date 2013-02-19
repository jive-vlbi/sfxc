#include "data_reader_blocking.h"
#include "vlba_reader.h"
#include "utils.h"
#include "backtrace.h"

VLBA_reader::
VLBA_reader(boost::shared_ptr<Data_reader> data_reader, Time ref_date_)
    : Input_data_format_reader(data_reader),
      debug_level_(NO_CHECKS),
      block_count_(0), DATA_RATE_(0), N(0), track(-1), mask(0)
{
  ref_jday = ref_date_.get_mjd();
  current_time_ = 0;
}

VLBA_reader::~VLBA_reader() {}

bool 
VLBA_reader::open_input_stream(Data_frame &data){
  SFXC_ASSERT(N > 0);
  if (!find_start_of_header(data)){
    if (eof())
      sfxc_abort("Could not find header before eof()");
    return false;
  }

  start_day_ = header.julian_day(0);
  int dif_jday = (start_day_ -  ref_jday % 1000); // the header containts jday % 1000
  current_jday = dif_jday >= 0 ? ref_jday + dif_jday : ref_jday + 1000 + dif_jday;
  SFXC_ASSERT(start_day_ == current_jday % 1000);

  start_time_.set_time_usec(current_jday, header.microseconds(0));
  current_time_.set_time_usec(current_jday, header.microseconds(0));
  std::cout << RANK_OF_NODE << " : Start of VLBA stream found t = " << get_current_time() << "\n";
  set_data_frame_info(data);
  find_fill_pattern(data);
  is_open_ = true;
  return true;
}

Time
VLBA_reader::goto_time(Data_frame &data, Time time) {
  if (!data_reader_->is_seekable()){
    // Reading from a socket, read one frame at a time
    while (time > get_current_time()) {
      if (!read_new_block(data))
        break;
    }
  } else if (time > get_current_time()){
    // Skip through data stream with 1 second steps
    const Time one_sec(1000000.);
    const Time t_one_byte((8 * 1000000.) / data_rate());
    Time delta_time = time - get_current_time();

    while(delta_time>=one_sec){
      // Read an integer number of frames
      size_t bytes_data_to_read = one_sec / t_one_byte - SIZE_VLBA_FRAME*N;
      SFXC_ASSERT(bytes_data_to_read %(SIZE_VLBA_FRAME*N)==0);

      int no_frames_to_read=bytes_data_to_read/(SIZE_VLBA_FRAME*N);
      size_t read_n_bytes = bytes_data_to_read + no_frames_to_read*N*(SIZE_VLBA_HEADER+SIZE_VLBA_AUX_HEADER);

      /// A blocking read operation. The operation is looping until the file
      /// is eof or the requested amount of data is retreived.
      size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );
      if ( byte_read != read_n_bytes) {
        std::cout << "Tried to read " << read_n_bytes << " but read " << byte_read << " instead\n";
        sfxc_abort("Couldn't read the requested amount of data.");
        return get_current_time();
      }

      // Need to read the data to check the header
      if (!read_new_block(data)) {
        DEBUG_MSG("Couldn't read data");
      }
      delta_time = time - get_current_time();
    }
    // Now read the last bit of data up to the requested time
    ssize_t bytes_data_to_read = delta_time / t_one_byte - SIZE_VLBA_FRAME*N;
    if(bytes_data_to_read>0){
      SFXC_ASSERT(bytes_data_to_read %(SIZE_VLBA_FRAME*N)==0);

      int no_frames_to_read=bytes_data_to_read/(SIZE_VLBA_FRAME*N);
      size_t read_n_bytes = bytes_data_to_read + no_frames_to_read*N*(SIZE_VLBA_HEADER+SIZE_VLBA_AUX_HEADER);

      size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );
      if ( byte_read != read_n_bytes) {
        std::cout << "Tried to read " << read_n_bytes << " but read " << byte_read << " instead\n";
        sfxc_abort("Couldn't read the requested amount of data.");
        return get_current_time();
      }

      // Need to read the data to check the header
      if (!read_new_block(data)) {
        DEBUG_MSG("Couldn't read data");
      }
    }  
  }
  return get_current_time();
}

std::string VLBA_reader::time_to_string(int64_t time) {
  // TODO This doesn't work, gives wrong time
  int milisecond = time % 1000;
  time /= 1000;
  int second = time % 60;
  time /= 60;
  int minute = time % 60;
  time /= 60;
  int hour = time % 24;
  time /= 24;
  int day = time;

  char time_str[40];
  snprintf(time_str,40, "%03dd%02dh%02dm%02ds%03dms",
           day, hour, minute, second, milisecond);
  return std::string(time_str);

}

bool VLBA_reader::read_new_block(Data_frame &data) {
  const int total_header_size = SIZE_VLBA_HEADER + SIZE_VLBA_AUX_HEADER;
  std::vector<value_type> &buffer = data.buffer->data;
  // Set to the right size
  if (buffer.size() != (SIZE_VLBA_FRAME*N))
    buffer.resize(SIZE_VLBA_FRAME*N);

  if (eof()) {
    current_time_ += time_between_headers();
    return false;
  }

  int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                     total_header_size*N, (char *)&buf_header[0] );
  // READ THE DATA FRAME  
  int result = Data_reader_blocking::get_bytes_s( data_reader_.get(), SIZE_VLBA_FRAME*N, (char *)&buffer[0] );
  if (result < 0) {
    DEBUG_MSG("FAILURE IN READING");
    current_time_ += time_between_headers();
    return false;
  } 

  // at last we read the complete header. Check it
  header.set_header(N, &buf_header[0]);
  if((!header.check_header(mask))&&(!resync_header(data))){
    current_time_ += time_between_headers(); // Could't find valid header before EOF
    return false;
  }
  if(header.julian_day(0) != current_jday % 1000)
    current_jday++;
  current_time_.set_time_usec(current_jday, header.microseconds(0));

  if (debug_level_ >= CHECK_PERIODIC_HEADERS) {
    if ((debug_level_ >= CHECK_ALL_HEADERS) ||
        ((++block_count_ % 100) == 0)) {
      header.check_header(mask);
      check_time_stamp(header);
      if (debug_level_ >= CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics(data)) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }
  set_data_frame_info(data);
  find_fill_pattern(data);

  return true;
}

bool VLBA_reader::resync_header(Data_frame &data) {
  const int max_read = RESYNC_MAX_DATA_FRAMES * size_data_block();
  int data_read = 0;
  generate_track_mask();
  // Find the next header in the input stream, NB: data already contains one VLBA block worth of input data
  std::cout << RANK_OF_NODE << " : Resync header, t = " << get_current_time() << "\n";

  unsigned char *buffer = &data.buffer->data[0];
  int bytes_read=0, header_start=0, nOnes=0;
  bool continue_searching = true;

  do{
    int byte = 0;
    while(byte < N*SIZE_VLBA_FRAME - 8*SIZE_VLBA_HEADER) {
      int shift = N * 32 - 1;
      while ((shift >= 0) && (buffer[byte+shift] & mask == mask))
        shift--;
      if (shift < 0) {
        // There is garanteed to be enough data in the buffer for a complete header (ignoring aux header)
        int header_start=byte;
        buf_header.resize((SIZE_VLBA_HEADER + SIZE_VLBA_AUX_HEADER) * N);
        memcpy(&buf_header[N * SIZE_VLBA_AUX_HEADER], buffer+header_start, N * SIZE_VLBA_HEADER);
        header.set_header(N, &buf_header[0]);
        int ndata_read = N * SIZE_VLBA_FRAME - header_start - N*SIZE_VLBA_HEADER;
        memmove(buffer, buffer + header_start + N*SIZE_VLBA_HEADER, ndata_read);
        if (header.check_header(mask)) {
          int to_read =  N*SIZE_VLBA_FRAME - ndata_read;
          int bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), to_read,
                                                             (char*)&data.buffer->data[ndata_read]);
          return true;
        }else if(data_read < max_read){
          // No valid header found, fill frame and continue
          int to_read =  N*SIZE_VLBA_FRAME - ndata_read;
          int bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), to_read,
                                                             (char *) &buffer[ndata_read]);
          data_read += bytes_read;
          byte = 0;
        }else{
          // Mamimum amount of data read
          std::cout << RANK_OF_NODE << " : Could not find VLBA header before EOF\n";
          return false;
        }
      }else{
        // Syncword not found, adjust index for partial match
        byte += 32 * N - 1;
        int jump = (buffer[byte] & mask == mask)? 32*N-2 : -1;
        byte -= jump;
      }
    }

    if(data_read < max_read){
      // Move the last half frame to the first half and read the remaining data
      size_t bytes_to_read = (N-1)*SIZE_VLBA_FRAME + SIZE_VLBA_FRAME/2;
      memcpy(buffer, buffer + bytes_to_read, SIZE_VLBA_FRAME/2);
      char *data = (char*)buffer + SIZE_VLBA_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), bytes_to_read, data);
      data_read += bytes_read;
      generate_track_mask(); // Try new track mask
    }else
      continue_searching = false;

  }while(continue_searching);
  std::cout << "Could find new sync word\n";
  return false;
}

bool VLBA_reader::check_time_stamp(VLBA_header &header) {
  const int64_t us_per_day=(int64_t)24*60*60*1000000;
  int64_t time_in_us = header.microseconds(0);
  int64_t delta_time;
  // the header only contains the last 3 digits of the julian day
  if(header.julian_day(0)>=start_day_)
    delta_time = (header.julian_day(0)-start_day_)*us_per_day + time_in_us - start_time_.get_time_usec();
  else
    delta_time = (1000+header.julian_day(0)-start_day_)*us_per_day + time_in_us - start_time_.get_time_usec();

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    SFXC_ASSERT(delta_time > 0);
  }
  int64_t computed_TBR =
    (int64_t)data_reader_->data_counter()*1000000/delta_time;

  if (computed_TBR != data_rate()) {
    return false;
  }
  return true;
}


bool
VLBA_reader::check_track_bit_statistics(Data_frame &data) {
  unsigned char* vlba_block = &data.buffer->data[SIZE_VLBA_HEADER];
  double track_bit_statistics[N*8];
  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track]=0;
  }

  for (int i=160; i<SIZE_VLBA_FRAME; i++) {
    for (int track=0; track<N*8; track++) {
      track_bit_statistics[track] += (vlba_block[i] >> track) &1;
    }
  }

  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track] /= SIZE_VLBA_FRAME;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }
  return true;
}


bool VLBA_reader::eof() {
  return data_reader_->eof();
}


void
VLBA_reader::set_parameters(const Input_node_parameters &input_node_param) {
  N = input_node_param.n_tracks / 8;
  int tbr = input_node_param.track_bit_rate;
  DATA_RATE_ = (tbr * N * 8);
  SFXC_ASSERT(DATA_RATE_ > 0);
  time_between_headers_ = Time(N * 8 * SIZE_MK5A_FRAME / (data_rate() / 1000000.));
  offset = input_node_param.offset;
  generate_track_mask();
}

void VLBA_reader::set_data_frame_info(Data_frame &data) {
  data.start_time = get_current_time();
}

int VLBA_reader::data_rate() const {
  SFXC_ASSERT(DATA_RATE_ > 0);
  return DATA_RATE_;
}

void VLBA_reader::generate_track_mask(){
  // Mask that selects atleast one track in each byte
  track = (track + 1)%8;
  mask = 1 << track;
  // if N=1 we match 2 tracks 
  if(N==1){
    int track2;
    do{
      track2 = rand()%8;
    }while (track == track2);
    mask |= 1 << track2;
  }
}

bool VLBA_reader::find_start_of_header(Data_frame &data){
  const int max_read = 16 * SIZE_VLBA_FRAME;  // Amount of data to read before giving up
  int data_read = SIZE_VLBA_FRAME / 2; // We start by reading half a bloc

  data.buffer->data.resize(SIZE_VLBA_FRAME);
  unsigned char *buffer_start = (unsigned char *)&data.buffer->data[0];

  { // Read half a block
    size_t bytes_to_read = SIZE_VLBA_FRAME/2;
    char *data = (char *)buffer_start+SIZE_VLBA_FRAME/2;

    int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, data);

    if( byte_read != bytes_to_read ){
      std::cout << "Unable to read enough bytes of data, cannot find a vlba header before the end-of-file\n";
      return false;
    }
  }

  int header_start=-1;
  while(data_read < max_read){
    // Move the last half to the first half and read size_vlba_frame/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_VLBA_FRAME/2, SIZE_VLBA_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_VLBA_FRAME/2;
      char *data = (char*)buffer_start+SIZE_VLBA_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), bytes_to_read, data);
      data_read += bytes_read;
    }

    int byte = 0;
    while(byte<SIZE_MK5A_FRAME - 8*SIZE_VLBA_HEADER) {
      int shift = N * 32 - 1;
      while ((shift >= 0) && (buffer_start[byte + shift] & mask == mask)){
        shift--;
      }
      if (shift < 0) {
        // There is garanteed to be enough data in the buffer for a complete header
        // First frame we ignore the AUX header, its not used anyway
        int header_start=byte;
        buf_header.resize((SIZE_VLBA_HEADER + SIZE_VLBA_AUX_HEADER) * N);
        memcpy(&buf_header[N * SIZE_VLBA_AUX_HEADER], buffer_start+header_start, N * SIZE_VLBA_HEADER);
        header.set_header(N, &buf_header[0]);
        int ndata_read = SIZE_VLBA_FRAME - header_start - N*SIZE_VLBA_HEADER;
        memmove(buffer_start, buffer_start + header_start + N*SIZE_VLBA_HEADER, ndata_read);
        if (header.check_header(mask)) {
          data.buffer->data.resize(N * SIZE_VLBA_FRAME);
          buffer_start = &data.buffer->data[0];
          int to_read =  SIZE_VLBA_FRAME - ndata_read + (N-1)*SIZE_VLBA_FRAME;
          int bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), to_read,
                                                             (char*)&data.buffer->data[ndata_read]);
          return true;
        }else if(data_read < max_read){
          // No valid header found, fill frame and continue
          Data_reader_blocking::get_bytes_s(data_reader_.get(), SIZE_VLBA_FRAME - ndata_read, (char *) &buffer_start[ndata_read]);
          data_read += ndata_read;
          byte = 0;
        }else{
          // Mamimum amount of data read
          std::cout << RANK_OF_NODE << " : Could not find VLBA header before EOF\n";
          return false;
        }
      }else{
        // Syncword not found, adjust index for partial match
        byte += 32 * N - 1;
        int jump = (buffer_start[byte] & mask == mask)? 32*N-2 : -1;
        byte -= jump;
      }
    }
  }
  std::cout << RANK_OF_NODE << " : Could not find vlba header before EOF\n";
  return false;
}

