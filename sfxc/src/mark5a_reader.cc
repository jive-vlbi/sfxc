#include "data_reader_blocking.h"
#include "mark5a_reader.h"
#include "utils.h"
#include "backtrace.h"

#ifdef SFXC_CHECK_INVALID_SAMPLES
// Buffer to store the first header. This is needed because the headers are 
// when this define is set.
unsigned char saved_header[SIZE_MK5A_HEADER*16];
#endif

Mark5a_reader::
Mark5a_reader(boost::shared_ptr<Data_reader> data_reader,
              int N_,
              Data_frame &data, 
              int ref_year_,
              int ref_day_)
    : Input_data_format_reader(data_reader),
      debug_level_(CHECK_PERIODIC_HEADERS),
      block_count_(0), DATA_RATE_(0), N(N_),
      ref_year(ref_year_), ref_day(ref_day_) {
  Mark5a_header header(N);
  header.set_header(&data.buffer[0]);
  header.check_header();
  start_day_ = header.day(0);
  start_time_ = header.get_time_in_us(0); 
  // If no reference day is known then setting ref_day < 0 sets the current day as reference
  // this is needed e.g. for the mark5a_print_headers utility. 
  if(ref_day < 0)
    ref_day = start_day_;
  current_time_ = header.get_time_in_us(0);
  current_day_ = header.day(0);

  set_data_frame_info(data);
  us_per_day=(int64_t)24*60*60*1000000;
}

Mark5a_reader::~Mark5a_reader() {}

int64_t
Mark5a_reader::goto_time(Data_frame &data, int64_t us_time) {
  // Compute with times in microseconds to find the exact time of the data
  if (us_time < get_current_time()) {
    return get_current_time();
  } else if (us_time == get_current_time()) {
    return us_time;
  }

  // Read an integer number of frames
  int64_t delta_time=us_time-get_current_time();
  size_t read_n_bytes = 
             (delta_time*data_rate()/(8*1000000)) - SIZE_MK5A_FRAME*N;

  // Read an integer number of frames
  SFXC_ASSERT(read_n_bytes %(SIZE_MK5A_FRAME*N)==0);

  // TODO having a blocking read would be nice.
  // as well as a goto function.
  // size_t bytes_to_read = read_n_bytes;
  //while ( bytes_to_read > 0 && !data_reader_->eof() ) {
  //  size_t result = data_reader_->get_bytes(bytes_to_read,NULL);
  //  bytes_to_read -= result;
  //}
  /// A blocking read operation. The operation is looping until the file
  /// is eof or the requested amount of data is retreived.
  size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );

  if ( byte_read != read_n_bytes) {
    sfxc_abort("Couldn't read the requested amount of data.");
    return get_current_time();
  }

  // Need to read the data to check the header
  if (!read_new_block(data)) {
    DEBUG_MSG("Couldn't read data");
  }

  if (get_current_time() != us_time) {
    DEBUG_MSG("time:         " << us_time);
    DEBUG_MSG("current time: " << get_current_time());
    sleep(1);
    SFXC_ASSERT(get_current_time() == us_time);
  }

  return get_current_time();
}

int64_t Mark5a_reader::correct_raw_time(int64_t raw_time){
  // Convert time read from input stream to time relative to midnight on the reference day
  return raw_time + (current_day_-ref_day)*us_per_day;
}

int64_t Mark5a_reader::get_current_time() {
  return current_time_;
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
  // Set to the right size
  if (data.buffer.size() != (SIZE_MK5A_FRAME*N))
    data.buffer.resize(SIZE_MK5A_FRAME*N);

  int to_read = SIZE_MK5A_FRAME*N;
  unsigned char *buffer = (unsigned char *)&data.buffer[0];
  do {
    if (eof()) {
      current_time_ += time_between_headers();
      return false;
    }

    /// I'm not sure why we are increasing the time between header in case of
    /// failed reading. Maybe a kind of packet-missing detection.
    /// Todo check that.
    //int result = data_reader_->get_bytes(to_read, (char *)buffer);
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
  header.set_header(&data.buffer[0]);
  if((!header.is_valid())&&(!resync_header(data))){
    current_time_ += time_between_headers(); // Could't find valid header before EOF
    return false;
  }
  current_day_ = header.day(0);
  current_time_ = correct_raw_time(header.get_time_in_us(0));

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

  return true;
}

bool Mark5a_reader::resync_header(Data_frame &data) {
  // Find the next header in the input stream, NB: data already contains one mark5a block worth of input data

  char *buffer=(char *)&data.buffer[0];
  int bytes_read=0, header_start=0, nOnes=0;

  do{
    for(int i=0;i<N*SIZE_MK5A_FRAME;i++){
      if(buffer[i]==~(0))
        nOnes++;
      else{
        if (nOnes >= N*32){
          // Check if we found a header
          header_start = i - 64*N-nOnes; // There are 64bits before the sync word
          if(header_start >0){
            memmove(&buffer[0], &buffer[header_start],N*SIZE_MK5A_FRAME-header_start);
            bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), header_start,
                                                           &buffer[N*SIZE_MK5A_FRAME-header_start]);
            return true;
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
  int64_t time_in_us = header.get_time_in_us(0);
  int64_t delta_time =
    (header.day(0)-start_day_)*us_per_day + time_in_us - start_time_;

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
  unsigned char* mark5a_block = &data.buffer[0];
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
#ifdef SFXC_CHECK_INVALID_SAMPLES
  header.set_header(&saved_header[0]);
#else
  header.set_header(&data.buffer[0]);
#endif
  SFXC_ASSERT(header.check_header());

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
      result[curr_channel][track] =
        header.find_track(channel->sign_headstack-1,
                          channel->sign_tracks[i]);
      SFXC_ASSERT(header.headstack(result[curr_channel][track]) ==
                  channel->sign_headstack-1);
      SFXC_ASSERT(header.track(result[curr_channel][track]) ==
                  channel->sign_tracks[i]);
      track++;
      if (channel->bits_per_sample() == 2) {
        result[curr_channel][track] =
          header.find_track(channel->magn_headstack-1,
                            channel->magn_tracks[i]);
        SFXC_ASSERT(header.headstack(result[curr_channel][track]) ==
                    channel->magn_headstack-1);
        SFXC_ASSERT(header.track(result[curr_channel][track]) ==
                    channel->magn_tracks[i]);
        track++;
      }
    }
  }
  return result;
}

bool Mark5a_reader::eof() {
  return data_reader_->eof();
}


void
Mark5a_reader::set_parameters(const Input_node_parameters &input_node_param) {
  int tbr = input_node_param.track_bit_rate;
  DATA_RATE_ = (tbr * N * 8);
  SFXC_ASSERT(DATA_RATE_ > 0);
  current_time_=correct_raw_time(current_time_);
}

void Mark5a_reader::set_data_frame_info(Data_frame &data) {
  Mark5a_header header(N);
  header.set_header(&data.buffer[0]);
  data.start_time = correct_raw_time(header.get_time_in_us(0));
#ifdef SFXC_INVALIDATE_SAMPLES
  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = SIZE_MK5A_HEADER*N;

#ifdef SFXC_CHECK_INVALID_SAMPLES
  for(int i=0;i<SIZE_MK5A_HEADER*N;i++)
    data.buffer[i] = INVALID_PATTERN;
#endif

#else
  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = 0;

  // Randomize data
  // park_miller_random generates 31 random bits
  for(int i=0;i<SIZE_MK5A_HEADER*N;i++)
    data.buffer[i] = park_miller_random();
#endif
}

Mark5a_reader *
get_mark5a_reader(boost::shared_ptr<Data_reader> reader,
                  Mark5a_reader::Data_frame &data, int ref_year, int ref_day) {
  int n_tracks_8 = find_start_of_header(reader, data);
  if(n_tracks_8 <= 0)
    sfxc_abort("Couldn't find a mark5a header in the data file");
  Mark5a_header header(n_tracks_8);
  header.set_header(&data.buffer[0]);
#ifdef SFXC_CHECK_INVALID_SAMPLES
  memcpy(&saved_header[0], &data.buffer[0], SIZE_MK5A_HEADER*n_tracks_8);
#endif
  if(!header.checkCRC())
    sfxc_abort("Invalid crc-code in the mark5a data file");
  DEBUG_MSG("Mark5a reader found start of data at : y=" << header.year(0)
            << ", day = " << header.day(0) << ", time =" << header.get_time_in_us(0));
  return new Mark5a_reader(reader, n_tracks_8, data, ref_year, ref_day);
}

int Mark5a_reader::data_rate() const {
  SFXC_ASSERT(DATA_RATE_ > 0);
  return DATA_RATE_;
}

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         Mark5a_reader::Data_frame &data) {
  // We fill the "data" and then look for the header
  // if we don't find a header, read in another half block and continue.

  data.buffer.resize(SIZE_MK5A_FRAME);
  char *buffer_start = (char *)&data.buffer[0];

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

      //do {
      //  int read = reader->get_bytes(bytes_to_read, data);
      //  bytes_to_read -= read;
      //  data += read;
      //  SFXC_ASSERT_MSG(!reader->eof(),
      //                  "Didn't find a mark5a header before the end-of-file");
      //} while (bytes_to_read > 0);
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


            //int byte_to_read = header_start;
            //int byte_read;
            //while(byte_to_read > 0){
            //  byte_read = reader->get_bytes(byte_to_read,
            //                               );
            //  byte_to_read -= byte_read;
            //}
            int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                     header_start,
                                     buffer_start+SIZE_MK5A_FRAME-header_start);

            if (nTracks8 > 1) {
              data.buffer.resize(nTracks8*SIZE_MK5A_FRAME);
              buffer_start = (char *)&data.buffer[0];


              //reader->get_bytes((nTracks8-1)*SIZE_MK5A_FRAME,
              //                  buffer_start+SIZE_MK5A_FRAME);
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
