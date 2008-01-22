/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */


#include <assert.h>

#include "channel_extractor_mark4.h"
#include "mark4_header.h"
#include "log_writer_cout.h"

#include "utils.h"

// Templated by the type of the element from which the samples are extracted
// Either int32_t (n_head_stacks == 1) or int64_t (n_head_stacks == 2)
template <class Type>
class Channel_extractor_mark4_implementation {
public:
  typedef Channel_extractor_mark4::Debug_level  Debug_level;

  Channel_extractor_mark4_implementation(boost::shared_ptr<Data_reader> reader,
                                         char *first_data_block,
                                         bool insert_random_headers_,
                                         Debug_level debug_level);
  bool set_input_node_parameters(const Input_node_parameters &parameters);
  

  int goto_time(int64_t time);
  int64_t get_current_time();
  std::string time2string(int64_t time);

  // Writes a block of data to all output streams
  size_t get_bytes(std::vector< char* > &buff);

  bool eof();

  bool check_track_bit_statistics();  
  
  void print_header(Log_writer &writer, int track);

  Mark4_header<Type> &header() {
    return mark4_header;
  }

  // bits/second per track
  int track_bit_rate() const;

  // bits/second per channel
  int bit_rate(int channel) const;

  // Number of channels
  int n_channels();
  // Number of tracks
  int n_tracks(int channel);

  // Proceeds to the next block
  int read_new_block();

  // Returns the number of bytes per block that are returned for one channel.
  int number_of_bytes_per_block();
private:

  bool check_time_stamp();
  
  boost::shared_ptr<Data_reader> reader;
  
  /// Bit positions for the sign and magnitude bits, per channel
  std::vector< std::vector<int> > tracks;

  /// Insertion of random bits for the headers, to remove a false signal
  bool insert_random_headers; 

  /// The data
  Type block[SIZE_MK4_FRAME];
  
  Mark4_header<Type> mark4_header;
  
  int32_t start_day;
  int64_t start_microtime;

  Debug_level debug_level;
  int block_count;

  int _track_bit_rate;
};


Channel_extractor_mark4::
Channel_extractor_mark4(boost::shared_ptr<Data_reader> reader, 
                        bool insert_random_headers_,
                        Debug_level debug_level)
 : ch_extractor_8_tracks(NULL),
   ch_extractor_16_tracks(NULL),
   ch_extractor_32_tracks(NULL),
   ch_extractor_64_tracks(NULL)
{
  char block[SIZE_MK4_FRAME];
  total_tracks = find_header(block, reader);
  
  switch (total_tracks) {
  case 8:
    {
      ch_extractor_8_tracks = 
        new Channel_extractor_mark4_implementation<uint8_t>
        (reader, block, insert_random_headers_, debug_level);
      break;
    }
  case 16:
    {
      ch_extractor_16_tracks = 
        new Channel_extractor_mark4_implementation<uint16_t>
        (reader, block, insert_random_headers_, debug_level);
      break;
    }
  case 32:
    {
      ch_extractor_32_tracks = 
        new Channel_extractor_mark4_implementation<uint32_t>
        (reader, block, insert_random_headers_, debug_level);
      break;
    }
  case 64:
    {
      ch_extractor_64_tracks = 
        new Channel_extractor_mark4_implementation<uint64_t>
        (reader, block, insert_random_headers_, debug_level);
      break;
    }
  default: 
    {
      assert(false);
    }
  }
}

int 
Channel_extractor_mark4::find_header(char *buffer,
                                     boost::shared_ptr<Data_reader> reader) {
  // buffer is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the buffer and then look for the header
  // if we don't find a header, read in another half block and continue.
  size_t bytes_read = reader->get_bytes(SIZE_MK4_FRAME/2, buffer+SIZE_MK4_FRAME/2);
  assert (bytes_read == SIZE_MK4_FRAME/2);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(buffer, buffer+SIZE_MK4_FRAME/2, SIZE_MK4_FRAME/2);
    size_t bytes_read = reader->get_bytes(SIZE_MK4_FRAME/2, buffer+SIZE_MK4_FRAME/2);
    assert (bytes_read == SIZE_MK4_FRAME/2);


    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if (buffer[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if ((nOnes>0) && (nOnes%32 == 0)) {
          // make sure the begin of the header is in the buffer
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes*3;
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;
            switch (nTracks8) {
            case 1: 
              {
                Mark4_header<uint8_t> header;
                header.set_header((uint8_t*)(buffer+header_start));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                break;
              }
            case 2: 
              {
                Mark4_header<uint16_t> header;
                header.set_header((uint16_t*)(buffer+header_start));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                break;
              }
            case 4: 
              {
                Mark4_header<uint32_t> header;
                header.set_header((uint32_t*)(buffer+header_start));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                break;
              }
            case 8: 
              {
                Mark4_header<uint64_t> header;
                header.set_header((uint64_t*)(buffer+header_start));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                break;
              }
            default:
              {
                assert(false);
              }
            }
          }
        }
        nOnes=0;
      }
    }
  }
  if (header_start < 0) return -1;
  if (header_start == 0) return nTracks8*8;

  memmove(buffer, buffer+header_start, SIZE_MK4_FRAME-header_start);
  reader->get_bytes(header_start, buffer+SIZE_MK4_FRAME-header_start);

  return nTracks8*8;
}

int Channel_extractor_mark4::n_channels() {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->n_channels();
  case 16: return ch_extractor_16_tracks->n_channels();
  case 32: return ch_extractor_32_tracks->n_channels();
  case 64: return ch_extractor_64_tracks->n_channels();
  default: assert(false);
  }
  return false;
}

int Channel_extractor_mark4::n_tracks() {
  return total_tracks;
}

int Channel_extractor_mark4::track(int track) {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->header().track(track);
  case 16: return ch_extractor_16_tracks->header().track(track);
  case 32: return ch_extractor_32_tracks->header().track(track);
  case 64: return ch_extractor_64_tracks->header().track(track);
  default: assert(false);
  }
}

int Channel_extractor_mark4::headstack(int track) {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->header().headstack(track);
  case 16: return ch_extractor_16_tracks->header().headstack(track);
  case 32: return ch_extractor_32_tracks->header().headstack(track);
  case 64: return ch_extractor_64_tracks->header().headstack(track);
  default: assert(false);
  }
}

int 
Channel_extractor_mark4::goto_time(int64_t time) {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->goto_time(time);
  case 16: return ch_extractor_16_tracks->goto_time(time);
  case 32: return ch_extractor_32_tracks->goto_time(time);
  case 64: return ch_extractor_64_tracks->goto_time(time);
  default: assert(false);
  }
  return 0;
}
int64_t 
Channel_extractor_mark4::get_current_time() {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->get_current_time();
  case 16: return ch_extractor_16_tracks->get_current_time();
  case 32: return ch_extractor_32_tracks->get_current_time();
  case 64: return ch_extractor_64_tracks->get_current_time();
  default: assert(false);
  }
  return 0;
}

size_t 
Channel_extractor_mark4::get_bytes(std::vector< char* > &buff) {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->get_bytes(buff);
  case 16: return ch_extractor_16_tracks->get_bytes(buff);
  case 32: return ch_extractor_32_tracks->get_bytes(buff);
  case 64: return ch_extractor_64_tracks->get_bytes(buff);
  default: assert(false);
  }
  return 0;
}

bool Channel_extractor_mark4::eof() {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->eof();
  case 16: return ch_extractor_16_tracks->eof();
  case 32: return ch_extractor_32_tracks->eof();
  case 64: return ch_extractor_64_tracks->eof();
  default: assert(false);
  }
  return false;
}

void Channel_extractor_mark4::print_header(Log_writer &writer, int track) {
  switch (total_tracks) {
  case  8: 
    return ch_extractor_8_tracks->print_header(writer, track);
  case 16: 
    return ch_extractor_16_tracks->print_header(writer, track);
  case 32:
    return ch_extractor_32_tracks->print_header(writer, track);
  case 64:
    return ch_extractor_64_tracks->print_header(writer, track);
  default: assert(false);
  }
}

int Channel_extractor_mark4::track_bit_rate() const {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->track_bit_rate();
  case 16: return ch_extractor_16_tracks->track_bit_rate();
  case 32: return ch_extractor_32_tracks->track_bit_rate();
  case 64: return ch_extractor_64_tracks->track_bit_rate();
  default: assert(false);
  }
}

int Channel_extractor_mark4::bit_rate(int channel) const {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->bit_rate(channel);
  case 16: return ch_extractor_16_tracks->bit_rate(channel);
  case 32: return ch_extractor_32_tracks->bit_rate(channel);
  case 64: return ch_extractor_64_tracks->bit_rate(channel);
  default: assert(false);
  }
}
int Channel_extractor_mark4::number_of_bytes_per_block() {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->number_of_bytes_per_block();
  case 16: return ch_extractor_16_tracks->number_of_bytes_per_block();
  case 32: return ch_extractor_32_tracks->number_of_bytes_per_block();
  case 64: return ch_extractor_64_tracks->number_of_bytes_per_block();
  default: assert(false);
  }
}
int Channel_extractor_mark4::goto_next_block() {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->read_new_block();
  case 16: return ch_extractor_16_tracks->read_new_block();
  case 32: return ch_extractor_32_tracks->read_new_block();
  case 64: return ch_extractor_64_tracks->read_new_block();
  default: assert(false);
  }
}
bool 
Channel_extractor_mark4::set_input_node_parameters(const Input_node_parameters &param) {
  switch (total_tracks) {
  case  8: return ch_extractor_8_tracks->set_input_node_parameters(param);
  case 16: return ch_extractor_16_tracks->set_input_node_parameters(param);
  case 32: return ch_extractor_32_tracks->set_input_node_parameters(param);
  case 64: return ch_extractor_64_tracks->set_input_node_parameters(param);
  default: assert(false);
  }
}

/*********************************************************************
 * Implementation of the Channel_extractor for Mark 4 files (32 or 64 bit)
 *********************************************************************/

template <class Type>
Channel_extractor_mark4_implementation<Type>::
Channel_extractor_mark4_implementation(boost::shared_ptr<Data_reader> reader, 
                                       char *first_data_block,
                                       bool insert_random_headers_,
                                       Debug_level debug_level)
 : reader(reader),
   insert_random_headers(insert_random_headers_),
   debug_level(debug_level),
   block_count(0)
{ 
  memcpy(block, first_data_block, SIZE_MK4_FRAME);
  // Make sure the header starts on the first byte:
  size_t result = reader->get_bytes(SIZE_MK4_FRAME*(sizeof(Type)-1), 
                                   ((char *)block)+SIZE_MK4_FRAME);
  assert(result == SIZE_MK4_FRAME*(sizeof(Type)-1));
  
  mark4_header.set_header(block);
  mark4_header.check_header();
  start_day = mark4_header.day(0);
  start_microtime = mark4_header.get_time_in_ms(0);
  start_microtime = 
    start_microtime*1000 + mark4_header.microsecond(0, start_microtime);
  reader->reset_data_counter();
}

template <class Type>
bool
Channel_extractor_mark4_implementation<Type>::
set_input_node_parameters(const Input_node_parameters &parameters) {
  _track_bit_rate = parameters.track_bit_rate;
  tracks.resize(parameters.channels.size());
  int curr_channel =0;
  // Store a list of tracks: first magnitude (optional), then sign 
  for (Input_node_parameters::Channel_const_iterator channel = 
         parameters.channels.begin();
       channel != parameters.channels.end(); channel++, curr_channel++) {
    tracks[curr_channel].resize(channel->bits_per_sample() *
                                channel->sign_tracks.size());
    
    int track =0;
    for (size_t i=0; i<channel->sign_tracks.size(); i++) {
      tracks[curr_channel][track] =
        mark4_header.find_track(channel->sign_headstack-1,
                                channel->sign_tracks[i]);
      assert(mark4_header.headstack(tracks[curr_channel][track]) == 
             channel->sign_headstack-1);
      assert(mark4_header.track(tracks[curr_channel][track]) == 
             channel->sign_tracks[i]);
      track++;
      if (channel->bits_per_sample() == 2) {
        tracks[curr_channel][track] = 
          mark4_header.find_track(channel->magn_headstack-1,
                                  channel->magn_tracks[i]);
        assert(mark4_header.headstack(tracks[curr_channel][track]) == 
               channel->magn_headstack-1);
        assert(mark4_header.track(tracks[curr_channel][track]) == 
               channel->magn_tracks[i]);
        track++;
      }
    }
  }
  return true;
}

template <class Type>
int 
Channel_extractor_mark4_implementation<Type>::
goto_time(int64_t _time) {
  // convert time to microseconds:
  int64_t microtime = _time*1000 + mark4_header.microsecond(0, _time);

  int64_t current_microtime;
  current_microtime = mark4_header.get_time_in_ms(0);
  current_microtime = 
    current_microtime*1000 + mark4_header.microsecond(0, current_microtime);

  if (microtime < current_microtime) {
    std::cout << "time in past, current time is: " 
              << time2string(current_microtime) << std::endl;
    std::cout << "            requested time is: " 
              << time2string(microtime) << std::endl;
    return -1;
  } else if (microtime == current_microtime) {
    return 0;
  }

  size_t read_n_bytes = 
    (microtime-current_microtime)*track_bit_rate()*sizeof(Type)/1000000 -
    SIZE_MK4_FRAME*sizeof(Type);
  
  assert(read_n_bytes > 0);
  // Read an integer number of frames
  assert(read_n_bytes %(SIZE_MK4_FRAME*sizeof(Type))==0);

  size_t result = reader->get_bytes(read_n_bytes,NULL);
  if (result != read_n_bytes) {
    assert(false);
    return result;
  }

  // Need to read the data to check the header
  result = read_new_block();
  if (result != SIZE_MK4_FRAME) return read_n_bytes+result;

  if (get_current_time() != _time) {
    DEBUG_MSG("_time:        " << _time);
    DEBUG_MSG("current time: " << get_current_time());
    assert(get_current_time() == _time);
  }
  return 0;
}

template <class Type>
int64_t
Channel_extractor_mark4_implementation<Type>::
get_current_time() {
  return
    (mark4_header.day(0)-start_day)*24*60*60*1000 + 
    mark4_header.get_time_in_ms(0);
}

template <class Type>
std::string 
Channel_extractor_mark4_implementation<Type>::
time2string(int64_t time) {
  char time_str[80];
  time = time/1000;
  int ms = time % 1000;
  time = time/1000;
  int s = time % 60;
  time = time/60;
  int m = time % 60;
  time = time/60;
  int h = time;
  snprintf(time_str, 80, "%02dh%02dm%02ds%03dms", h, m, s, ms);
  return std::string(time_str);
}


template <class Type>
size_t
Channel_extractor_mark4_implementation<Type>::
get_bytes(std::vector< char* > &output_buffer) {
  assert(output_buffer.size() == tracks.size());

  // The fan out is the same for all channels
  int fan_out = tracks[0].size();
  int n_channels = output_buffer.size();

  for (int i=0; i<n_channels; i++) {
    memset(&output_buffer[i][0], 0, SIZE_MK4_FRAME*fan_out/8);
  }
  
  for (int channel =0; channel<n_channels; channel++) {
  
    if (output_buffer[channel] == NULL) continue;
    int data_position = 0; // Position in the mark4 data block
    size_t output_position = 0;
    if (insert_random_headers && false) {
      // there are 160 bits in the header
      while (data_position < 160) {
        output_buffer[channel][output_position] = // 8 random bits:
          irbit2() + 
          2*(irbit2() + 
             2*(irbit2() + 
                2*(irbit2() + 
                   2*(irbit2() + 
                      2*(irbit2() + 
                         2*(irbit2() + 
                            2*(irbit2())))))));
          output_position++;
        data_position+=8/fan_out;
      }
      assert(output_position == (size_t)(160/8)*fan_out);
    }
    while (data_position < SIZE_MK4_FRAME) {
      for (int bit=0; bit<8;) {
        assert(!tracks[channel].empty());
        for (size_t track_it=0; track_it<tracks[channel].size(); track_it++) {
          // insert the bit into the output buffer
          output_buffer[channel][output_position] |= 
            (((block[data_position]>>tracks[channel][track_it])&1) << bit);
          // Shift a position in the output buffer
          bit ++;
        }
        assert(bit <= 8);
        data_position++;
      }
      output_position++;
    }
    assert(output_position == (SIZE_MK4_FRAME*tracks[channel].size())/8);
  }
  return (SIZE_MK4_FRAME*tracks[0].size())/8;
}

  
template <class Type>
int
Channel_extractor_mark4_implementation<Type>::
read_new_block() {
  int result = reader->get_bytes(SIZE_MK4_FRAME*sizeof(Type),(char *)block)/sizeof(Type);
  if (result != SIZE_MK4_FRAME) {
    return result;
  }

  if (debug_level >= Channel_extractor_mark4::CHECK_PERIODIC_HEADERS) {
    if ((debug_level >= Channel_extractor_mark4::CHECK_ALL_HEADERS) ||
        ((++block_count % 100) == 0)) {
      mark4_header.check_header();
      check_time_stamp();
      if (debug_level >= Channel_extractor_mark4::CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics()) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }

  return result;
}

template <class Type>
bool
Channel_extractor_mark4_implementation<Type>::
check_time_stamp() {
  int64_t militime = mark4_header.get_time_in_ms(0);
  int64_t delta_time = 
    (mark4_header.day(0)-start_day)*24*60*60*1000000 + 
    militime*1000 + mark4_header.microsecond(0, militime)
    - start_microtime;

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    assert(delta_time > 0);
  }
  int64_t computed_TBR =
    (reader->data_counter()*1000000/(sizeof(Type)*delta_time));
  
  if (computed_TBR != track_bit_rate()) {
    return false;
  }
  return true;
}

template <class Type>
bool
Channel_extractor_mark4_implementation<Type>::
eof() {
  return reader->eof();
}

template <class Type>
int
Channel_extractor_mark4_implementation<Type>::
n_channels() {
  return tracks.size();
}

template <class Type>
bool
Channel_extractor_mark4_implementation<Type>::
check_track_bit_statistics() {
  double track_bit_statistics[sizeof(Type)*8];
  for (size_t track=0; track<sizeof(Type)*8; track++) {
    track_bit_statistics[track]=0;
  }
  
  for (int i=160; i<SIZE_MK4_FRAME; i++) {
    for (size_t track=0; track<sizeof(Type)*8; track++) {
      track_bit_statistics[track] += (block[i] >> track) &1;
    }
  }
  
  for (size_t track=0; track<sizeof(Type)*8; track++) {
    track_bit_statistics[track] /= SIZE_MK4_FRAME;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }    
  
  return true;
}

template <class Type>
void 
Channel_extractor_mark4_implementation<Type>::
print_header(Log_writer &writer, int track) {
//   mark4_header.print_binary_header(writer);
  writer << "time: " << mark4_header.get_time_str(track) << std::endl;
}

template <class Type>
int
Channel_extractor_mark4_implementation<Type>::
track_bit_rate() const {
  return _track_bit_rate;
}

template <class Type>
int
Channel_extractor_mark4_implementation<Type>::
bit_rate(int channel) const {
  return track_bit_rate()*tracks[channel].size();
}

template <class Type>
int
Channel_extractor_mark4_implementation<Type>::
number_of_bytes_per_block() {
  assert(!tracks.empty());
  return SIZE_MK4_FRAME*tracks[0].size()/8;
}

