#include "data_reader_blocking.h"
#include "vlba_reader.h"
#include "utils.h"
#include "backtrace.h"

VLBA_reader::
VLBA_reader(boost::shared_ptr<Data_reader> data_reader,
              int N_,
              Data_frame &data, Data_frame &header_, Data_frame &aux_header_)
    : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
block_count_(0), DATA_RATE_(0), N(N_), header(N_){
  // SET HEADER
  buf_header.resize(SIZE_VLBA_HEADER*N);
  memcpy(&buf_header[0], &header_.buffer[0], SIZE_VLBA_HEADER*N);
  buf_aux_header.resize(SIZE_VLBA_AUX_HEADER*N);
  memcpy(&buf_aux_header[0], &aux_header_.buffer[0], SIZE_VLBA_AUX_HEADER*N);
  header.set_header(&buf_header[0], &buf_aux_header[0]);
  DEBUG_MSG("CHECKING HDR");
  header.check_header();
  start_day_ = header.julian_day(0);
  start_time_ = header.microseconds(0);
  // initially use start_date as reference
  ref_jday = header.julian_day(0);
  current_time_ = correct_raw_time(header.microseconds(0));

  set_data_frame_info(data);
  us_per_day=(int64_t)24*60*60*1000000;
}

VLBA_reader::~VLBA_reader() {}

int64_t
VLBA_reader::goto_time(Data_frame &data, int64_t us_time) {
  // Compute with times in microseconds to find the exact time of the data
  if (us_time < get_current_time()) {
    return get_current_time();
  } else if (us_time == get_current_time()) {
    return us_time;
  }

  // Read an integer number of frames
  int64_t delta_time=us_time-get_current_time();
  size_t bytes_data_to_read = 
          (size_t)(delta_time*data_rate()/(8*1000000)) - SIZE_VLBA_FRAME*N;
  SFXC_ASSERT(bytes_data_to_read %(SIZE_VLBA_FRAME*N)==0);

  int no_frames_to_read=bytes_data_to_read/(SIZE_VLBA_FRAME*N);
  size_t read_n_bytes = bytes_data_to_read + no_frames_to_read*N*(SIZE_VLBA_HEADER+SIZE_VLBA_AUX_HEADER);

  /// A blocking read operation. The operation is looping until the file
  /// is eof or the requested amount of data is retreived.
  size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), read_n_bytes, NULL );
  if ( byte_read != read_n_bytes) {
    std::cout << "Tried to read " << read_n_bytes << " but read " << byte_read << " instead\n";
    SFXC_ASSERT_MSG(false,
                    "Couldn't read the requested amount of data.");
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

int64_t VLBA_reader::correct_raw_time(int64_t raw_time){
  // Convert time read from input stream to time relative to midnight on the reference day
  int cur_jday = header.julian_day(0);
  int64_t ret_val;
  if(cur_jday >= ref_jday)
    ret_val = raw_time + (cur_jday-ref_jday)*us_per_day;
  else
    ret_val = raw_time + (1000+cur_jday-ref_jday)*us_per_day;

  return ret_val;
}

int64_t VLBA_reader::get_current_time() {

  return current_time_;
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
  // Set to the right size
  if (data.buffer.size() != (SIZE_VLBA_FRAME*N))
    data.buffer.resize(SIZE_VLBA_FRAME*N);

  if (eof()) {
    current_time_ += time_between_headers();
    return false;
  }

  int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                       SIZE_VLBA_HEADER*N, (char *)&buf_header[0] );
  // READ THE DATA FRAME  
  int result = Data_reader_blocking::get_bytes_s( data_reader_.get(), SIZE_VLBA_FRAME*N, (char *)&data.buffer[0] );
  if (result < 0) {
    DEBUG_MSG("FAILURE IN READING");
    current_time_ += time_between_headers();
    return false;
  } 

  byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                      SIZE_VLBA_AUX_HEADER*N,
                                                      (char *)&buf_aux_header[0] );

  // at least we read the complete header. Check it
  header.set_header(&buf_header[0],&buf_aux_header[0]);
  if((!header.is_valid())&&(!resync_header(data))){
    current_time_ += time_between_headers(); // Could't find valid header before EOF
    return false;
  }
  current_time_ = correct_raw_time(header.microseconds(0));

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

bool VLBA_reader::resync_header(Data_frame &data) {
  // Find the next header in the input stream, NB: data already contains one VLBA block worth of input data

  char *buffer=(char *)&data.buffer[0];
  int bytes_read=0, header_start=0, nOnes=0;

  do{
    for(int i=0;i<N*SIZE_VLBA_FRAME;i++){
      if(buffer[i]==~(0))
        nOnes++;
      else{
        if (nOnes >= N*32){
          // Check if we found a header
          header_start = i - nOnes; 
          if(header_start >0){
            memmove(&buffer[0], &buffer[header_start],N*SIZE_VLBA_FRAME-header_start);
            bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), header_start,
                                                           &buffer[N*SIZE_VLBA_FRAME-header_start]);
            return true;
          }
        } 
        nOnes=0;
      }
    }
    header_start = N*SIZE_VLBA_FRAME-nOnes; 
    memcpy(&buffer[0], &buffer[header_start],N*SIZE_VLBA_FRAME-header_start);
    bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), header_start,
                                                   &buffer[N*SIZE_VLBA_FRAME-header_start]);
  }while(bytes_read>0);

  std::cout << "Could find new sync word\n";
  return false;
}

bool VLBA_reader::check_time_stamp(VLBA_header &header) {
  int64_t time_in_us = header.microseconds(0);
  int64_t delta_time;
  // the header only contains the last 3 digits of the julian day
  if(header.julian_day(0)>=start_day_)
    delta_time = (header.julian_day(0)-start_day_)*us_per_day + time_in_us - start_time_;
  else
    delta_time = (1000+header.julian_day(0)-start_day_)*us_per_day + time_in_us - start_time_;

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
  unsigned char* vlba_block = &data.buffer[SIZE_VLBA_HEADER];
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


std::vector< std::vector<int> >
VLBA_reader::get_tracks(const Input_node_parameters &input_node_param,
                          Data_frame &data) {
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

      SFXC_ASSERT(header.track(result[curr_channel][track]) ==
                  channel->sign_tracks[i]);
      track++;
      if (channel->bits_per_sample() == 2) {
        result[curr_channel][track] =
          header.find_track(channel->magn_headstack-1,
                            channel->magn_tracks[i]);

        SFXC_ASSERT(header.track(result[curr_channel][track]) ==
                    channel->magn_tracks[i]);
        track++;
      }
    }
  }

  return result;
}

bool VLBA_reader::eof() {
  return data_reader_->eof();
}


void
VLBA_reader::set_parameters(const Input_node_parameters &input_node_param) {
  int tbr = input_node_param.track_bit_rate;
  ref_jday = (mjd(1,1,input_node_param.start_year) + input_node_param.start_day -1 )%1000;
  current_time_=correct_raw_time(current_time_);
  DATA_RATE_ = (tbr * N * 8);
  SFXC_ASSERT(DATA_RATE_ > 0);
}

void VLBA_reader::set_data_frame_info(Data_frame &data) {
  data.start_time = correct_raw_time(header.microseconds(0));

  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = 0;
}

VLBA_reader *
get_vlba_reader(boost::shared_ptr<Data_reader> reader,
                  VLBA_reader::Data_frame &data) {
  
  VLBA_reader::Data_frame header, aux_header;
  int n_tracks_8 = find_start_of_vlba_header(reader, data, header, aux_header);
  SFXC_ASSERT_MSG(n_tracks_8 > 0,
                  "Couldn't find a vlba header in the data file");
  VLBA_header test_header(n_tracks_8);
  test_header.set_header(&header.buffer[0],&aux_header.buffer[0]);

  SFXC_ASSERT_MSG(test_header.checkCRC(),
                  "Invalid crc-code in the vlba data file");

  return new VLBA_reader(reader, n_tracks_8, data, header, aux_header);
}

int VLBA_reader::data_rate() const {
  SFXC_ASSERT(DATA_RATE_ > 0);
  return DATA_RATE_;
}

int find_start_of_vlba_header(boost::shared_ptr<Data_reader> reader,
                              VLBA_reader::Data_frame &data,
                              VLBA_reader::Data_frame &header,
                              VLBA_reader::Data_frame &aux_header) {
  // We fill the "data" and then look for the header
  // if we don't find a header, read in another half block and continue.
  data.buffer.resize(SIZE_VLBA_FRAME);
  char *buffer_start = (char *)&data.buffer[0];

  { // Read half a block
    size_t bytes_to_read = SIZE_VLBA_FRAME/2;
    char *data = (char *)buffer_start+SIZE_VLBA_FRAME/2;

    int byte_read = Data_reader_blocking::get_bytes_s( reader.get(), bytes_to_read, data);

    if( byte_read != bytes_to_read ){
      DEBUG_MSG("Unable to read enough bytes of data, cannot find a vlba header before the end-of-file");
      SFXC_ASSERT(false && "We should exit");
    }
  }

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read size_vlba_frame/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_VLBA_FRAME/2, SIZE_VLBA_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_VLBA_FRAME/2;
      char *data = (char*)buffer_start+SIZE_VLBA_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(), bytes_to_read, data);
    }

    for (int byte=0; (byte<SIZE_VLBA_FRAME) && (header_start<0); byte++) {
      if ((char)buffer_start[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        // Because the syncword is 32 ones, we should find atleast 32 (more depending on #tracks)
        if (nOnes>=32) {
          // make sure that really found the start of the header
          int header_start=byte-nOnes;
          if(header_start>1){
            // We found a complete header
            nTracks8 = nOnes/32;

            // Store the header and get the first track8 worth of data
            int ndata_read=SIZE_VLBA_FRAME-header_start-nTracks8*SIZE_VLBA_HEADER;
            header.buffer.resize(nTracks8*SIZE_VLBA_HEADER);
            memcpy(&header.buffer[0], buffer_start+header_start, nTracks8*SIZE_VLBA_HEADER);
            memmove(buffer_start, buffer_start+header_start+nTracks8*SIZE_VLBA_HEADER, ndata_read);
            int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),SIZE_VLBA_FRAME-ndata_read,
                                                                          (char*)&data.buffer[ndata_read]);

            // read the remaining data
            if (nTracks8 > 1) {
              data.buffer.resize(nTracks8*SIZE_VLBA_FRAME);

              buffer_start = (char *)&data.buffer[0];
              int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                                  (nTracks8-1)*SIZE_VLBA_FRAME,
                                                   buffer_start+SIZE_VLBA_FRAME);
            }
            // read the aux header
              aux_header.buffer.resize(nTracks8*SIZE_VLBA_AUX_HEADER);
              buffer_start = (char *)&aux_header.buffer[0];
              bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                                   nTracks8*SIZE_VLBA_AUX_HEADER,
                                                   buffer_start);
            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}