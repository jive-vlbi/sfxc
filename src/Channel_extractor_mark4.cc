/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Channel_extractor_mark4.h"
#include <genFunctions.h>
#include "Mark4_header.h"

#include <assert.h>


// Templated by the type of the element from which the samples are extracted
// Either INT32 (n_head_stacks == 1) or INT64 (n_head_stacks == 2)
template <class T>
class Channel_extractor_mark4_implementation {
public:
  typedef Channel_extractor_mark4::DEBUG_LEVEL  DEBUG_LEVEL;

  Channel_extractor_mark4_implementation(boost::shared_ptr<Data_reader> reader, 
                                         char *first_data_block,
                                         StaP &staPrms,
                                         bool insert_random_headers_,
                                         DEBUG_LEVEL debug_level);

  int goto_time(INT64 time);
  INT64 get_current_time();
  std::string time2string(INT64 time);

  size_t do_get_bytes(size_t nBytes, char *buff);

  size_t get_samples(size_t nSamples, double *bit_samples, 
                     const double *val_array);
  
  bool eof();

  bool check_track_bit_statistics();  
  
  void print_header(Log_writer &writer, int track);

  Mark4_header<T> &header() {
    return mark4_header;
  }
private:

  bool increase_current_position_in_block();

  int read_new_block();
  
  bool check_time_stamp();
  
  boost::shared_ptr<Data_reader> reader;
  
  /// The number of samples per 32/64 bit integer
  int fan_out;

  /// Number of bits per sample (sign or sign+magn)
  int n_bits_per_sample;
  
  /// Bit positions for the sign and magnitude bits 
  std::vector<int> tracks;

  /// Insertion of random bits for the headers, to remove a false signal
  bool insert_random_headers; 

  /// The data
  T block[frameMk4];
  
  /// Read pointer in the data (an index)
  int curr_pos_in_block;
  
  Mark4_header<T> mark4_header;
  
  INT32 start_day;
  INT64 start_microtime;
  INT32 TBR;
  DEBUG_LEVEL debug_level;
  int block_count;
};


Channel_extractor_mark4::
Channel_extractor_mark4(boost::shared_ptr<Data_reader> reader, 
                        StaP &staPrms, 
                        bool insert_random_headers_,
                        DEBUG_LEVEL debug_level)
 : Channel_extractor(),
   ch_extractor_8_tracks(NULL),
   ch_extractor_16_tracks(NULL),
   ch_extractor_32_tracks(NULL),
   ch_extractor_64_tracks(NULL)
{
  char block[frameMk4];
  n_tracks = find_header(block, reader);

  switch (n_tracks) {
  case 8:
    {
      ch_extractor_8_tracks = 
        new Channel_extractor_mark4_implementation<uint8_t>
        (reader, block, staPrms, insert_random_headers_, debug_level);
      break;
    }
  case 16:
    {
      ch_extractor_16_tracks = 
        new Channel_extractor_mark4_implementation<uint16_t>
        (reader, block, staPrms, insert_random_headers_, debug_level);
      break;
    }
  case 32:
    {
      ch_extractor_32_tracks = 
        new Channel_extractor_mark4_implementation<uint32_t>
        (reader, block, staPrms, insert_random_headers_, debug_level);
      break;
    }
  case 64:
    {
      ch_extractor_64_tracks = 
        new Channel_extractor_mark4_implementation<uint64_t>
        (reader, block, staPrms, insert_random_headers_, debug_level);
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
  size_t bytes_read = reader->get_bytes(frameMk4/2, buffer+frameMk4/2);
  assert (bytes_read == frameMk4/2);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(buffer, buffer+frameMk4/2, frameMk4/2);
    size_t bytes_read = reader->get_bytes(frameMk4/2, buffer+frameMk4/2);
    assert (bytes_read == frameMk4/2);


    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=64; (byte<frameMk4-64*8) && (header_start<0); byte++) {
      if (buffer[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if ((nOnes>0) && (nOnes%32 == 0)) {
          // make sure the begin of the header is in the buffer
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes*3;
          if (header_start >= 0) {
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

  memmove(buffer, buffer+header_start, frameMk4-header_start);
  reader->get_bytes(header_start, buffer+frameMk4-header_start);

  return nTracks8*8;
}

int Channel_extractor_mark4::number_of_tracks() {
  return n_tracks;
}

int Channel_extractor_mark4::track(int track) {
  switch (n_tracks) {
  case  8: return ch_extractor_8_tracks->header().track(track);
  case 16: return ch_extractor_16_tracks->header().track(track);
  case 32: return ch_extractor_32_tracks->header().track(track);
  case 64: return ch_extractor_64_tracks->header().track(track);
  default: assert(false);
  }
}

int Channel_extractor_mark4::headstack(int track) {
  switch (n_tracks) {
  case  8: return ch_extractor_8_tracks->header().headstack(track);
  case 16: return ch_extractor_16_tracks->header().headstack(track);
  case 32: return ch_extractor_32_tracks->header().headstack(track);
  case 64: return ch_extractor_64_tracks->header().headstack(track);
  default: assert(false);
  }
}

int 
Channel_extractor_mark4::goto_time(INT64 time) {
  switch (n_tracks) {
  case  8: return ch_extractor_8_tracks->goto_time(time);
  case 16: return ch_extractor_16_tracks->goto_time(time);
  case 32: return ch_extractor_32_tracks->goto_time(time);
  case 64: return ch_extractor_64_tracks->goto_time(time);
  default: assert(false);
  }
  return 0;
}
INT64 
Channel_extractor_mark4::get_current_time() {
  switch (n_tracks) {
  case  8: return ch_extractor_8_tracks->get_current_time();
  case 16: return ch_extractor_16_tracks->get_current_time();
  case 32: return ch_extractor_32_tracks->get_current_time();
  case 64: return ch_extractor_64_tracks->get_current_time();
  default: assert(false);
  }
  return 0;
}

size_t 
Channel_extractor_mark4::
get_samples(size_t nSamples, double *bit_samples, const double *val_array) {
  switch (n_tracks) {
  case  8: 
    return ch_extractor_8_tracks->get_samples(nSamples,bit_samples,val_array);
  case 16: 
    return ch_extractor_16_tracks->get_samples(nSamples,bit_samples,val_array);
  case 32:
    return ch_extractor_32_tracks->get_samples(nSamples,bit_samples,val_array);
  case 64:
    return ch_extractor_64_tracks->get_samples(nSamples,bit_samples,val_array);
  default: assert(false);
  }
  return 0;
}

size_t 
Channel_extractor_mark4::do_get_bytes(size_t nBytes, char *buff) {
  switch (n_tracks) {
  case  8: 
    return ch_extractor_8_tracks->do_get_bytes(nBytes, buff);
  case 16: 
    return ch_extractor_16_tracks->do_get_bytes(nBytes, buff);
  case 32:
    return ch_extractor_32_tracks->do_get_bytes(nBytes, buff);
  case 64:
    return ch_extractor_64_tracks->do_get_bytes(nBytes, buff);
  default: assert(false);
  }
  return 0;
}

bool Channel_extractor_mark4::eof() {
  switch (n_tracks) {
  case  8: 
    return ch_extractor_8_tracks->eof();
  case 16: 
    return ch_extractor_16_tracks->eof();
  case 32:
    return ch_extractor_32_tracks->eof();
  case 64:
    return ch_extractor_64_tracks->eof();
  default: assert(false);
  }
  return false;
}

void Channel_extractor_mark4::print_header(Log_writer &writer, int track) {
  switch (n_tracks) {
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

/*********************************************************************
 * Implementation of the Channel_extractor for Mark 4 files (32 or 64 bit)
 *********************************************************************/

template <class T>
Channel_extractor_mark4_implementation<T>::
Channel_extractor_mark4_implementation(boost::shared_ptr<Data_reader> reader, 
                                       char *first_data_block,
                                       StaP &staPrms, 
                                       bool insert_random_headers_,
                                       DEBUG_LEVEL debug_level)
 : reader(reader),
   fan_out(staPrms.get_fo()), 
   n_bits_per_sample(staPrms.get_bps()),
   insert_random_headers(insert_random_headers_),
   curr_pos_in_block(0),
   TBR(staPrms.get_tbr()),
   debug_level(debug_level),
   block_count(0)
{ 
  memcpy(block, first_data_block, frameMk4);
  // Make sure the header starts on the first byte:
  size_t result = reader->get_bytes(frameMk4*(sizeof(T)-1), 
                                   ((char *)block)+frameMk4);
  assert(result == frameMk4*(sizeof(T)-1));
  
  mark4_header.set_header(block);
  mark4_header.check_header();
  
  start_day = mark4_header.day(0);
  start_microtime = mark4_header.get_microtime(0);
  reader->reset_data_counter();

  // Store a list of tracks: first magnitude (optional), then sign 
  tracks.resize(n_bits_per_sample*fan_out);
  assert(tracks.size() <= 8);
  int track = 0;
  for (int i=0; i<fan_out; i++) {
    if (n_bits_per_sample > 1) {
      tracks[track] = mark4_header.find_track(staPrms.get_headstack_magn(),
                                              staPrms.get_magnBS()[i]);
      assert(mark4_header.headstack(tracks[track]) == 
             staPrms.get_headstack_magn());
      assert(mark4_header.track(tracks[track]) == 
             staPrms.get_magnBS()[i]);
//       if (mark4_header.is_sign(tracks[track]))
//         std::cout << "Track " << tracks[track] << " is not a magn track" 
//                   << std::endl;

      track++;
    }
    tracks[track] = mark4_header.find_track(staPrms.get_headstack_sign(),
                                            staPrms.get_signBS()[i]);
    assert(mark4_header.headstack(tracks[track]) == 
           staPrms.get_headstack_sign());
    assert(mark4_header.track(tracks[track]) == 
           staPrms.get_signBS()[i]);
//     if (mark4_header.is_magn(tracks[track]))
//       std::cout << "Track " << tracks[track] << " is not a sign track" 
//                 << std::endl;
    track++;
  }
}

template <class T>
int 
Channel_extractor_mark4_implementation<T>::
goto_time(INT64 time) {
  INT64 current_time = get_current_time();
  if (time < current_time) {
    std::cout << "time in past, current time is: " 
              << time2string(current_time) << std::endl;
    std::cout << "            requested time is: " 
              << time2string(time) << std::endl;
    return -1;
  } else if (time == current_time) {
    return 0;
  }
  size_t read_n_bytes = (time-current_time) * sizeof(T)* TBR - 
                         frameMk4*sizeof(T);
  
  if (read_n_bytes == 0) {
    return 0;
  }
  size_t result = reader->get_bytes(read_n_bytes,NULL);
  if (result != read_n_bytes) return result;

  // Need to read the data to check the header
  read_new_block();

  assert(get_current_time() == time);
  // reset read pointer:
  curr_pos_in_block = 0;
  return 0;
}

template <class T>
INT64
Channel_extractor_mark4_implementation<T>::
get_current_time() {
  return mark4_header.get_microtime(tracks[0]);
}

template <class T>
std::string 
Channel_extractor_mark4_implementation<T>::
time2string(INT64 time) {
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


template <class T>
size_t
Channel_extractor_mark4_implementation<T>::
do_get_bytes(size_t nOutputBytes, char *output_buffer) {
  UINT64 bytes_processed = 0;
  
  while (bytes_processed < nOutputBytes) {
    // Fill the output buffer
    if (insert_random_headers && (curr_pos_in_block < 160)) {
      // We reading data from the header, generate random data
      // Get the number of bytes to read from the header:
      int nbytes_from_header = 
        min(nOutputBytes-bytes_processed, 
            ((160-curr_pos_in_block)*tracks.size())/8);
      assert (nbytes_from_header > 0);
      curr_pos_in_block += (nbytes_from_header*8)/tracks.size();

      for (;nbytes_from_header > 0; nbytes_from_header--) {
        output_buffer[bytes_processed] = // 8 random bits:
          irbit2() + 
          2*(irbit2() + 
             2*(irbit2() + 
                2*(irbit2() + 
                   2*(irbit2() + 
                      2*(irbit2() + 
                         2*(irbit2() + 
                            2*(irbit2())))))));
        bytes_processed ++;
      }
      assert(curr_pos_in_block < frameMk4);
    } else {
      // We are reading from the data block.
      // Filled from least to most significant bit
      int nbytes_from_data_block = 
        min(nOutputBytes-bytes_processed, 
            ((frameMk4-curr_pos_in_block)*tracks.size())/8);
      for (;nbytes_from_data_block > 0; nbytes_from_data_block--) {
        output_buffer[bytes_processed] = 0;
        for (int bit=0; bit<8;) {
          for (size_t track_it=0; track_it<tracks.size(); track_it++) {
            // insert the bit into the output buffer
            output_buffer[bytes_processed] |= 
              ((( block[curr_pos_in_block]>>tracks[track_it] ) & 1) << bit);
            // Shift a position in the output buffer
            bit ++;
          }
          curr_pos_in_block++;
        }
        bytes_processed++;
      }
      if (curr_pos_in_block == frameMk4) {
        int result = read_new_block();
        if (result != frameMk4) {
          // Could not read a new block: terminating
          curr_pos_in_block = frameMk4-1;
          return bytes_processed;
        }
        curr_pos_in_block = 0;
      }
      assert(curr_pos_in_block < frameMk4);
    }
  }
  return bytes_processed;
}

template <class T>
size_t
Channel_extractor_mark4_implementation<T>::
get_samples(size_t nSamples, double *samples, const double *val_array) {
  assert(nSamples%(fan_out) == 0);

  size_t samples_processed = 0;

  if (n_bits_per_sample == 1) {
    // Not yet implemented
    assert (false);
  } else {
    while (samples_processed < nSamples) {
      // skip every second position!
      int bit_sample;
      for (size_t track_it=0; track_it<tracks.size(); track_it+=2) {
        //get sign and magnitude bit for all channels
        // we need to multiply with 2 since tracks[track_it+1] can be zero
        // and a >> of a negative value does not work
        if ((curr_pos_in_block < 160) && insert_random_headers) {
          bit_sample = irbit2() + 2*irbit2();
        } else {
          // set sign and magnitude:
          bit_sample =
            (( block[curr_pos_in_block]>> tracks[track_it] ) & 1)
            +
            (( block[curr_pos_in_block]>>tracks[track_it+1] ) & 1)*2;
        }
        samples[samples_processed] = val_array[bit_sample];

        samples_processed++;
      }
      if (!increase_current_position_in_block()) {
        // End of data
        return samples_processed;
      }
    }
  }

  return samples_processed;
}  
  
template <class T>
bool
Channel_extractor_mark4_implementation<T>::
increase_current_position_in_block() {
  curr_pos_in_block++;
  if (curr_pos_in_block == frameMk4) {
    int result = read_new_block();
    if (result != frameMk4) {
      // Could not read a new block: terminating
      curr_pos_in_block = frameMk4-1;
      return false;
    }
    curr_pos_in_block = 0;
  }
  return true;
}
  
template <class T>
int
Channel_extractor_mark4_implementation<T>::
read_new_block() {
  int result = reader->get_bytes(frameMk4*sizeof(T),(char *)block)/sizeof(T);
  if (result != frameMk4) {
    return result;
  }

  if (debug_level >= Channel_extractor_mark4::CHECK_PERIODIC_HEADERS) {
    if ((debug_level >= Channel_extractor_mark4::CHECK_ALL_HEADERS) ||
        ((++block_count % 100) == 0)) {
      mark4_header.check_header();
      check_time_stamp();
      if (Channel_extractor_mark4::CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics()) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }

  return result;
}

template <class T>
bool
Channel_extractor_mark4_implementation<T>::
check_time_stamp() {
  double delta_time = 
    mark4_header.get_microtime_difference(start_day, start_microtime, tracks[0])/1000000.;
  
  double computed_TBR = (reader->data_counter()*8/1000000.) / 
                        (delta_time * sizeof(T) * 8);
  
  if (computed_TBR != TBR) {
    std::cout << "Change in time: " << computed_TBR-TBR << std::endl;
    return false;
  }
  return true;
}

template <class T>
bool
Channel_extractor_mark4_implementation<T>::
eof() {
  return reader->eof();
}

template <class T>
bool
Channel_extractor_mark4_implementation<T>::
check_track_bit_statistics() {
  double track_bit_statistics[sizeof(T)*8];
  for (size_t track=0; track<sizeof(T)*8; track++) {
    track_bit_statistics[track]=0;
  }
  
  for (int i=160; i<frameMk4; i++) {
    for (size_t track=0; track<sizeof(T)*8; track++) {
      track_bit_statistics[track] += (block[i] >> track) &1;
    }
  }
  
  for (size_t track=0; track<sizeof(T)*8; track++) {
    track_bit_statistics[track] /= frameMk4;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }    
  
  return true;
}

template <class T>
void 
Channel_extractor_mark4_implementation<T>::
print_header(Log_writer &writer, int track) {
//   mark4_header.print_binary_header(writer);
  writer << "time: " << mark4_header.get_time_str(track) << std::endl;
}
