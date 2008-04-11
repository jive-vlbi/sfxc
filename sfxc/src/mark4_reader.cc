#include "mark4_reader.h"
#include "mark4_header.h"
#include "backtrace.h"

Mark4_reader::
Mark4_reader(boost::shared_ptr<Data_reader> data_reader,
             int N_,
             unsigned char *buffer,
             unsigned char *mark4_block)
    : data_reader_(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    block_count_(0), N(N_) {
  // fill the first mark4 block
  memmove(mark4_block, buffer, SIZE_MK4_FRAME*sizeof(unsigned char));
  int bytes_to_read = SIZE_MK4_FRAME*(N - sizeof(unsigned char));
  char *data = ((char *)mark4_block) + SIZE_MK4_FRAME*sizeof(unsigned char);
  while (bytes_to_read > 0) {
    int read = data_reader->get_bytes(bytes_to_read, data);
    assert(read >= 0);
    bytes_to_read -= read;
    data += read;
  }

  Mark4_header header(N);
  header.set_header(mark4_block);
  header.check_header();
  start_day_ = header.day(0);
  start_time_ = header.get_time_in_us(0);
  current_time_ = header.get_time_in_us(0);
}


Mark4_reader::~Mark4_reader() {}

int64_t
Mark4_reader::goto_time(unsigned char *mark4_block, int64_t us_time) {
  // Compute with times in microseconds to find the exact time of the data
  if (us_time < get_current_time()) {
    std::cout << "time in past, current time is: " << time_to_string(get_current_time()) << std::endl;
    std::cout << "            requested time is: " << time_to_string(us_time) << std::endl;
    return get_current_time()/1000;
  } else if (us_time == get_current_time()) {
    return us_time;
  }

  size_t read_n_bytes =
    (us_time-get_current_time())*MARK4_TRACK_BIT_RATE*N/1000000 -
    SIZE_MK4_FRAME*N;

  // Read an integer number of frames
  assert(read_n_bytes %(SIZE_MK4_FRAME*N)==0);

  // TODO having a blocking read would be nice.
  // as well as a goto function.
  size_t bytes_to_read = read_n_bytes;
  while ( bytes_to_read > 0 && !data_reader_->eof() ) {
    size_t result = data_reader_->get_bytes(bytes_to_read,NULL);
    bytes_to_read -= result;
  }

  if ( bytes_to_read != 0 ) {
    assert(false);
    return get_current_time();
  }

  // Need to read the data to check the header
  if (!read_new_block(mark4_block)) {
    assert(false);
    return get_current_time();
  }

  if (get_current_time() != us_time) {
    DEBUG_MSG("time:        " << us_time);
    DEBUG_MSG("current time: " << get_current_time());
    assert(get_current_time() == us_time);
  }

  return get_current_time();
}

int64_t Mark4_reader::get_current_time() {
  return current_time_;
}


std::string Mark4_reader::time_to_string(int64_t time) {
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

bool Mark4_reader::read_new_block(unsigned char *mark4_block) {
  int to_read = SIZE_MK4_FRAME*N;
  unsigned char *buffer = (unsigned char *)mark4_block;
  do {
    if (eof()) {
      DEBUG_MSG("EOF");
      current_time_ += time_between_headers();
      return false;
    }
    int result = data_reader_->get_bytes(to_read, (char *)buffer);
    if (result < 0) {
      current_time_ += time_between_headers();
      return false;
    } else if (result == 0) {
      DEBUG_MSG("ZERO");
    }
    to_read -= result;
    buffer += result;
  } while (to_read > 0);

  // at least we read the complete header. Check it
  Mark4_header header(N);
  header.set_header(mark4_block);
  current_time_ = header.get_time_in_us(0);

  if (debug_level_ >= CHECK_PERIODIC_HEADERS) {
    if ((debug_level_ >= CHECK_ALL_HEADERS) ||
        ((++block_count_ % 100) == 0)) {
      header.check_header();
      check_time_stamp(header);
      if (debug_level_ >= CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics(mark4_block)) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }

  return true;
}



bool Mark4_reader::check_time_stamp(Mark4_header &header) {
  int64_t time_in_us = header.get_time_in_us(0);
  int64_t delta_time =
    (header.day(0)-start_day_)*24*60*60*1000000 + time_in_us - start_time_;

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    assert(delta_time > 0);
  }
  int64_t computed_TBR =
    (data_reader_->data_counter()*1000000/(N*delta_time));

  if (computed_TBR != MARK4_TRACK_BIT_RATE) {
    return false;
  }
  return true;
}


bool
Mark4_reader::check_track_bit_statistics(unsigned char *mark4_block) {
  double track_bit_statistics[N*8];
  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track]=0;
  }

  for (int i=160; i<SIZE_MK4_FRAME; i++) {
    for (int track=0; track<N*8; track++) {
      track_bit_statistics[track] += (mark4_block[i] >> track) &1;
    }
  }

  for (int track=0; track<N*8; track++) {
    track_bit_statistics[track] /= SIZE_MK4_FRAME;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }
  return true;
}



std::vector< std::vector<int> >
Mark4_reader::get_tracks(const Input_node_parameters &input_node_param,
                         unsigned char *mark4_block) {
  Mark4_header header(N);
  header.set_header(mark4_block);
  assert(header.check_header());

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
      assert(header.headstack(result[curr_channel][track]) ==
             channel->sign_headstack-1);
      assert(header.track(result[curr_channel][track]) ==
             channel->sign_tracks[i]);
      track++;
      if (channel->bits_per_sample() == 2) {
        result[curr_channel][track] =
          header.find_track(channel->magn_headstack-1,
                            channel->magn_tracks[i]);
        assert(header.headstack(result[curr_channel][track]) ==
               channel->magn_headstack-1);
        assert(header.track(result[curr_channel][track]) ==
               channel->magn_tracks[i]);
        track++;
      }
    }
  }

  return result;
}

bool Mark4_reader::eof() {
  return data_reader_->eof();
}

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         unsigned char first_block[]) {
  // first_block is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the first_block and then look for the header
  // if we don't find a header, read in another half block and continue.
  size_t bytes_to_read = SIZE_MK4_FRAME/2;
  char *data = (char *)first_block+SIZE_MK4_FRAME/2;
  do {
    int read = reader->get_bytes(bytes_to_read, data);
    bytes_to_read -= read;
    data += read;
  } while (bytes_to_read > 0);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(first_block, first_block+SIZE_MK4_FRAME/2, SIZE_MK4_FRAME/2);

    size_t bytes_to_read = SIZE_MK4_FRAME/2;
    char *data = (char*)first_block+SIZE_MK4_FRAME/2;
    do {
      int read = reader->get_bytes(bytes_to_read, data);
      bytes_to_read -= read;
      data += read;
    } while (bytes_to_read > 0);


    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if ((char)first_block[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if (nOnes>=32) {
          // make sure the begin of the header is in the first_block
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes - 64*(nOnes/32);
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;

            memmove(first_block, first_block+header_start,
                    SIZE_MK4_FRAME-header_start);
            reader->get_bytes(header_start,
                              (char *)first_block+SIZE_MK4_FRAME-header_start);

            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}

Mark4_reader_interface *
get_mark4_reader(boost::shared_ptr<Data_reader> reader,
                 unsigned char *first_block) {

  int n_tracks_8 = find_start_of_header(reader, first_block);
  Mark4_header header(n_tracks_8);
  header.set_header(first_block);
  if (!header.checkCRC()) {
    assert(false);
    return NULL;
  }
  return new Mark4_reader(reader, n_tracks_8, first_block, first_block);
}

