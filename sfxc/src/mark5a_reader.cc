#include "mark5a_reader.h"
#include "utils.h"
#include "backtrace.h"

Mark5a_reader::
Mark5a_reader(boost::shared_ptr<Data_reader> data_reader,
              int N_,
              Data_frame &data)
    : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
block_count_(0), DATA_RATE_(0), N(N_) {
  Mark5a_header header(N);
  header.set_header(&data.buffer[0]);
  header.check_header();
  start_day_ = header.day(0);
  start_time_ = header.get_time_in_us(0);
  current_time_ = header.get_time_in_us(0);

  set_data_frame_info(data);
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

  size_t read_n_bytes =
    (us_time-get_current_time())*data_rate()/(8*1000000) -
    SIZE_MK5A_FRAME*N;

  // Read an integer number of frames
  SFXC_ASSERT(read_n_bytes %(SIZE_MK5A_FRAME*N)==0);

  // TODO having a blocking read would be nice.
  // as well as a goto function.
  size_t bytes_to_read = read_n_bytes;
  while ( bytes_to_read > 0 && !data_reader_->eof() ) {
    size_t result = data_reader_->get_bytes(bytes_to_read,NULL);
    bytes_to_read -= result;
  }

  if ( bytes_to_read != 0 ) {
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
    int result = data_reader_->get_bytes(to_read, (char *)buffer);
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
  current_time_ = header.get_time_in_us(0);

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



bool Mark5a_reader::check_time_stamp(Mark5a_header &header) {
  int64_t time_in_us = header.get_time_in_us(0);
  int64_t delta_time =
    (header.day(0)-start_day_)*24*60*60*1000000 + time_in_us - start_time_;

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    SFXC_ASSERT(delta_time > 0);
  }
  int64_t computed_TBR =
    (data_reader_->data_counter()*1000000/(delta_time));

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
  header.set_header(&data.buffer[0]);
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
}

void Mark5a_reader::set_data_frame_info(Data_frame &data) {
  Mark5a_header header(N);
  header.set_header(&data.buffer[0]);
  data.start_time = header.get_time_in_us(0);

#ifdef SFXC_INVALIDATE_SAMPLES
  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = SIZE_MK5A_HEADER*N;

#ifdef SFXC_CHECK_INVALID_SAMPLES
  input_element_.data().buffer[i] = value_type(0);
#endif

#else
  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = 0;

  // Randomize data
  // park_miller_random generates 31 random bits
  input_element_.data().buffer[i] = (value_type)park_miller_random();
#endif
}

Mark5a_reader *
get_mark5a_reader(boost::shared_ptr<Data_reader> reader,
                  Mark5a_reader::Data_frame &data) {
  int n_tracks_8 = find_start_of_header(reader, data);
  SFXC_ASSERT_MSG(n_tracks_8 > 0,
                  "Couldn't find a mark5a header in the data file");
  Mark5a_header header(n_tracks_8);
  header.set_header(&data.buffer[0]);
  SFXC_ASSERT_MSG(header.checkCRC(),
                  "Invalid crc-code in the mark5a data file");

  return new Mark5a_reader(reader, n_tracks_8, data);
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
    do {
      int read = reader->get_bytes(bytes_to_read, data);
      bytes_to_read -= read;
      data += read;
      SFXC_ASSERT_MSG(!reader->eof(),
                      "Didn't find a mark5a header before the end-of-file");
    } while (bytes_to_read > 0);
  }

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk5a/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_MK5A_FRAME/2, SIZE_MK5A_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_MK5A_FRAME/2;
      char *data = (char*)buffer_start+SIZE_MK5A_FRAME/2;
      do {
        int read = reader->get_bytes(bytes_to_read, data);
        bytes_to_read -= read;
        data += read;
        SFXC_ASSERT_MSG(!reader->eof(),
                        "Didn't find a mark5a header before the end-of-file");
      } while (bytes_to_read > 0);
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
            reader->get_bytes(header_start,
                              buffer_start+SIZE_MK5A_FRAME-header_start);
            if (nTracks8 > 1) {
              data.buffer.resize(nTracks8*SIZE_MK5A_FRAME);
              buffer_start = (char *)&data.buffer[0];
              reader->get_bytes((nTracks8-1)*SIZE_MK5A_FRAME,
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
