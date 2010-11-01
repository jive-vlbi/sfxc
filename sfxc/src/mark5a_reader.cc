#include <limits>
#include "data_reader_blocking.h"
#include "mark5a_reader.h"
#include "utils.h"
#include "backtrace.h"

Mark5a_reader::
Mark5a_reader(boost::shared_ptr<Data_reader> data_reader,
              int N_,
              Data_frame &data, 
              Time ref_time_)
    : Input_data_format_reader(data_reader),
      debug_level_(NO_CHECKS),
      block_count_(0), DATA_RATE_(0), N(N_) {
  int ref_year, ref_day;
  ref_time_.get_date(ref_year, ref_day);
  Mark5a_header header(N);
  header.set_header(&data.buffer->data[0]);
  header.check_header();
  std::cout << RANK_OF_NODE << " : Mark5a reader found start of data at : y=" << header.year(0)
            << ", day = " << header.day(0) << ", time =" << header.get_time_in_us(0) << "\n";
  start_day_ = header.day(0);
  current_mjd_ = mjd(1, 1, ref_year) + start_day_ - 1;
  start_time_.set_time_usec(current_mjd_, header.get_time_in_us(0)); 
  current_day_ = header.day(0);
  current_time_.set_time_usec(current_mjd_, header.get_time_in_us(0));

  set_data_frame_info(data);
  find_fill_pattern(data);
}

Mark5a_reader::~Mark5a_reader() {}

Time
Mark5a_reader::goto_time(Data_frame &data, Time time) {
  // Compute with times in microseconds to find the exact time of the data
  if (time < get_current_time()) {
    return get_current_time();
  } else if (time == get_current_time()) {
    return time;
  }

  // first skip through the file in 1 second steps.
  const Time one_sec(1000000.);
  const Time t_one_frame(8*N*SIZE_MK5A_FRAME / (data_rate() / 1000000.));
  const size_t max_blocks_to_read = std::numeric_limits<size_t>::max() / (SIZE_MK5A_FRAME*N);
  Time delta_time = time - get_current_time();
  while(delta_time>=one_sec){
    // Read an integer number of frames
    size_t n_blocks = std::min((size_t)(delta_time / t_one_frame)/2, max_blocks_to_read);
    size_t read_n_bytes =  (n_blocks - 1) * SIZE_MK5A_FRAME*N;

    // A blocking read operation. The operation is looping until the file
    // is eof or the requested amount of data is retreived.
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );
    if (byte_read != read_n_bytes)
      return current_time_;

    // Need to read the data to check the header
    if (!read_new_block(data)) {
      DEBUG_MSG("Couldn't read data");
    }
    delta_time = time - get_current_time();
  }
  // Now read the last bit of data up to the requested time
  int n_blocks = (int)(delta_time / t_one_frame);
  if(n_blocks > 0){
    size_t read_n_bytes =  (n_blocks - 1) * SIZE_MK5A_FRAME*N;
    //SFXC_ASSERT(read_n_bytes %(SIZE_MK5A_FRAME*N)==0);
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );
    if (byte_read != read_n_bytes)
      return current_time_;

    // Need to read the data to check the header
    if (!read_new_block(data)) {
      DEBUG_MSG("Couldn't read data");
    }
  }
  if (get_current_time() != time) {
    // When jumping to the start of the scan, it can happen that we don't end up exactly
    // at us_time, because the station might have started recording late.
    DEBUG_MSG("Attempted to jump to time " << time << ", but found timestamp" << get_current_time());
  }

  return get_current_time();
}

std::string Mark5a_reader::time_to_string(int64_t time) {
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

bool Mark5a_reader::read_new_block(Data_frame &data) {
  std::vector<value_type> &databuffer = data.buffer->data;
  // Set to the right size
  if (databuffer.size() != (SIZE_MK5A_FRAME*N))
    databuffer.resize(SIZE_MK5A_FRAME*N);

  int to_read = SIZE_MK5A_FRAME*N;
  unsigned char *buffer = (unsigned char *)&databuffer[0];
  do {
    if (eof()) {
      current_time_ += time_between_headers();
      return false;
    }
    int result = Data_reader_blocking::get_bytes_s( data_reader_.get(), to_read, (char*)buffer );

    if (result < 0) {
      DEBUG_MSG("FAILURE IN READING");
      current_time_ += time_between_headers();
      return false;
    } else if (result == 0) {}
    to_read -= result;
    buffer += result;
  } while (to_read > 0);

  // at least we read the complete header. Check it
  Mark5a_header header(N);
  header.set_header(&databuffer[0]);
  if((!header.check_header())&&(!resync_header(data))){
    current_time_ += time_between_headers(); // Could't find valid header before EOF
    return false;
  }
  if (header.day(0) == 0 && header.get_time_in_us(0) == 0) {
      current_time_ += time_between_headers(); // Could't find valid header before EOF
      return false;
  }

  // check if we are crossing a day boundary
  if(header.day(0) != current_day_){
    current_mjd_++;
    current_day_ = header.day(0);
  }
  current_time_.set_time_usec(current_mjd_, header.get_time_in_us(0));

  if (debug_level_ >= CHECK_PERIODIC_HEADERS) {
    if ((debug_level_ >= CHECK_ALL_HEADERS) ||
        ((++block_count_ % 100) == 0)) {
      header.check_header();
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

bool Mark5a_reader::resync_header(Data_frame &data) {
  // Find the next header in the input stream, NB: data already contains one mark5a block worth of input data

  char *buffer=(char *)&data.buffer->data[0];
  int bytes_read=0, header_start=0, nOnes=0;
  // start by reading half a header, otherwise if the resync was triggered by a crc error we'll find the previous header
  memcpy(&buffer[0], &buffer[N*SIZE_MK5A_FRAME/2], N*SIZE_MK5A_FRAME/2);
  Data_reader_blocking::get_bytes_s(data_reader_.get(), N*SIZE_MK5A_FRAME/2, &buffer[N*SIZE_MK5A_FRAME/2]);

  do{
    for(int i=0;i<N*SIZE_MK5A_FRAME;i++){
      if(buffer[i]==~(0))
        nOnes++;
      else{
        if (nOnes >= N*32){
          // Check if we found a header
          header_start = i - 64*N-nOnes; // There are 64bits before the sync word
          if(header_start >= 0){
            memmove(&buffer[0], &buffer[header_start],N*SIZE_MK5A_FRAME-header_start);
            bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), header_start,
                                                           &buffer[N*SIZE_MK5A_FRAME-header_start]);
	    Mark5a_header header(N);
            header.set_header((unsigned char *)&buffer[0]);

            if(header.check_header())
	      return true;
            else
              return resync_header(data);
          }
        } 
        nOnes=0;
      }
    }
    header_start = N*SIZE_MK5A_FRAME-nOnes - 64*N; // There are 64bits before the sync word
 
    memcpy(&buffer[0], &buffer[header_start],N*SIZE_MK5A_FRAME-header_start);
    bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), header_start,
                                                   &buffer[N*SIZE_MK5A_FRAME-header_start]);
  }while(bytes_read>0);

  std::cout << "Couldn't find new sync word before EOF\n";
  return false;
}

bool Mark5a_reader::check_time_stamp(Mark5a_header &header) {
  const int64_t us_per_day = 24*60*60*1000000LL;
  int64_t run_time = (int64_t)(header.get_time_in_us(0) - start_time_.get_time_usec());
  int64_t delta_time =
    (header.day(0)-start_day_)*us_per_day + run_time;

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
Mark5a_reader::check_track_bit_statistics(Data_frame &data) {
  unsigned char* mark5a_block = &data.buffer->data[0];
  double track_bit_statistics[N*8];
  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track]=0;
  }

  for (int i=160; i<SIZE_MK5A_FRAME; i++) {
    for (int track=0; track<N*8; track++) {
      track_bit_statistics[track] += (mark5a_block[i] >> track) &1;
    }
  }

  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track] /= SIZE_MK5A_FRAME;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }
  return true;
}

std::vector< std::vector<int> >
Mark5a_reader::get_tracks(const Input_node_parameters &input_node_param,
                          Data_frame &data) {
  Mark5a_header header(N);
  header.set_header(&data.buffer->data[0]);
  SFXC_ASSERT(header.check_header());

  std::vector< std::vector<int> > result;

  result.resize(input_node_param.channels.size());
  int curr_channel =0;
  // Store a list of tracks: first sign, then magn(optional)
  for (Input_node_parameters::Channel_const_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++, curr_channel++) {
    result[curr_channel].resize(channel->bits_per_sample() *
                                channel->sign_tracks.size());

    int track =0;
    for (size_t i=0; i<channel->sign_tracks.size(); i++) {
      result[curr_channel][track] =
        header.find_track(channel->sign_headstack-1,
                          channel->sign_tracks[i]);
      if(result[curr_channel][track] < 0)
        return get_standard_track_mapping(input_node_param, data);
      track++;
      if (channel->bits_per_sample() == 2) {
        result[curr_channel][track] =
          header.find_track(channel->magn_headstack-1,
                            channel->magn_tracks[i]);
        if(result[curr_channel][track] < 0)
          return get_standard_track_mapping(input_node_param, data);
        track++;
      }
    }
  }
  return result;
}

std::vector< std::vector<int> >
Mark5a_reader::get_standard_track_mapping(const Input_node_parameters &input_node_param,
                                          Data_frame &data) {
  std::cout << RANK_OF_NODE << " : WARNING - Using standard mark5a track mapping\n";
  std::vector< std::vector<int> > result;

  result.resize(input_node_param.channels.size());
  int curr_channel =0;
  // Store a list of tracks: first magnitude (optional), then sign
  for (Input_node_parameters::Channel_const_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++, curr_channel++) {
    result[curr_channel].resize(channel->bits_per_sample() *
                                channel->sign_tracks.size());

    int track =0;
    for (size_t i=0; i<channel->sign_tracks.size(); i++) {
      result[curr_channel][track] = 32*(channel->sign_headstack-1)+(channel->sign_tracks[i]-2);
      track++;
      if (channel->bits_per_sample() == 2) {
        result[curr_channel][track] = 32*(channel->magn_headstack-1)+(channel->magn_tracks[i]-2);
        track++;
      }
    }
  }
  return result;
}

void
Mark5a_reader::set_parameters(const Input_node_parameters &input_node_param) {
  int tbr = input_node_param.track_bit_rate;
  DATA_RATE_ = (tbr * N * 8);
  SFXC_ASSERT(DATA_RATE_ > 0);
  time_between_headers_ = Time(N * 8 * SIZE_MK5A_FRAME / (data_rate() / 1000000.));
}

void Mark5a_reader::set_data_frame_info(Data_frame &data) {
  Mark5a_header header(N);
  header.set_header(&data.buffer->data[0]);
  data.start_time = current_time_;
#ifdef SFXC_INVALIDATE_SAMPLES
  data.invalid.resize(1);
  data.invalid[0].invalid_begin = 0;
  data.invalid[0].nr_invalid = SIZE_MK5A_HEADER*N;

#ifdef SFXC_CHECK_INVALID_SAMPLES
  for(int i=0;i<SIZE_MK5A_HEADER*N;i++)
    data.buffer[i] = INVALID_PATTERN;
#endif

#else

  // Randomize data
  // park_miller_random generates 31 random bits
  for(int i=0;i<SIZE_MK5A_HEADER*N;i++)
    data.buffer[i] = park_miller_random();
#endif
}

Mark5a_reader *
get_mark5a_reader(boost::shared_ptr<Data_reader> reader,
                  Mark5a_reader::Data_frame &data, Time ref_date) {
  int n_tracks_8;
  bool header_correct;
  bool first_msg=true;
  do{
    n_tracks_8 = find_start_of_header(reader, data);
    if(n_tracks_8 <= 0)
      sfxc_abort("Couldn't find a mark5a header in the data file");
    Mark5a_header header(n_tracks_8);
    header.set_header(&data.buffer->data[0]);
    header_correct = header.checkCRC();
    if((first_msg)&&(!header_correct)){
      std::cout << RANK_OF_NODE 
                << " Warning : Invalid crc-code in the mark5a data file, further warnings are supressed.\n";
      first_msg=false;
    }
  }while(!header_correct);
  return new Mark5a_reader(reader, n_tracks_8, data, ref_date);
}

int Mark5a_reader::data_rate() const {
  SFXC_ASSERT(DATA_RATE_ > 0);
  return DATA_RATE_;
}

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         Mark5a_reader::Data_frame &data) {
  // We fill the "data" and then look for the header
  // if we don't find a header, read in another half block and continue.

  data.buffer->data.resize(SIZE_MK5A_FRAME);
  char *buffer_start = (char *)&data.buffer->data[0];
  
  { // Read half a block
    size_t bytes_to_read = SIZE_MK5A_FRAME/2;
    char *data = (char *)buffer_start+SIZE_MK5A_FRAME/2;

    int byte_read = Data_reader_blocking::get_bytes_s( reader.get(), bytes_to_read, data);

    if( byte_read != bytes_to_read ){
      sfxc_abort("Unable to read enough bytes of data, cannot find a mark5a header before the end-of-file");
    }

  }

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk5a/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_MK5A_FRAME/2, SIZE_MK5A_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_MK5A_FRAME/2;
      char *data = (char*)buffer_start+SIZE_MK5A_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(), bytes_to_read, data);

    }

    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK5A_FRAME-64*8) && (header_start<0); byte++) {
      if ((char)buffer_start[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if (nOnes>=32) {
          // make sure the begin of the header is in the first_block
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes - 64*(nOnes/32);
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;

            memmove(buffer_start, buffer_start+header_start,
                    SIZE_MK5A_FRAME-header_start);

            int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                     header_start,
                                     buffer_start+SIZE_MK5A_FRAME-header_start);

            if (nTracks8 > 1) {
              data.buffer->data.resize(nTracks8*SIZE_MK5A_FRAME);
              buffer_start = (char *)&data.buffer->data[0];

	      int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                                  (nTracks8-1)*SIZE_MK5A_FRAME,
                                                   buffer_start+SIZE_MK5A_FRAME);
            }
            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}
