#include <limits>
#include <stdio.h>
#include "data_reader_blocking.h"
#include "mark5b_reader.h"

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
              Data_frame &data, Time ref_date)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    time_between_headers_(0.), sample_rate(0), frame_nr_valid(true)
{
  // Initialize the crc16 lookup table
  gen_crc16();

  current_jday = ref_date.get_mjd();
  start_day_ = 0;
  start_time_ = 0;
}


Mark5b_reader::~Mark5b_reader() {}

bool 
Mark5b_reader::open_input_stream(Data_frame &data){
  if(!read_new_block(data)){
    if(!resync_header(data, 0))
       return false;
  }

  start_day_ = current_header.julian_day();
  start_time_ = current_time_;
  std::cout << RANK_OF_NODE << "Start of Mark5b data at jday=" << start_day_
            << ", time = " << start_time_ << "\n";
  is_open_ = true;
  return true;
}

Time
Mark5b_reader::goto_time(Data_frame &data, Time us_time) {
  SFXC_ASSERT(current_header.check());
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
  current_time_ = get_current_time();

  if (us_time <= current_time_) return current_time_;

  const size_t size_mk5b_block =
    (SIZE_MK5B_HEADER+SIZE_MK5B_FRAME)*SIZE_MK5B_WORD;
  const size_t max_blocks_to_read = std::numeric_limits<size_t>::max() /
                                    (size_mk5b_block * N_MK5B_BLOCKS_TO_READ);

  // first skip through the file in 1 second steps.
  const Time one_sec(1000000.);
  Time delta_time = us_time - current_time_;
  while (delta_time >= one_sec){
    SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
    size_t n_blocks = std::min((size_t)(delta_time/time_between_headers_)/2, max_blocks_to_read);
    // Don't read the last header, to be able to check whether we are at the right time
    size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block;
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
    if (bytes_to_read != byte_read)
      return current_time_;

    // Read last block:
    read_new_block(data);
    delta_time = us_time - current_time_;
  }
  // Now read the last bit of data up to the requested time
  int n_blocks = (int)round(delta_time / time_between_headers_);
  if(n_blocks>0){
    // Don't read the last header, to be able to check whether we are at the right time
    size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block;
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
    if (bytes_to_read != byte_read)
      return current_time_;

    read_new_block(data);
    if((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) != 0)
      return resync_header(data, 0);
  }
  current_time_ = get_current_time();

  return current_time_;
}

Time Mark5b_reader::get_current_time() {
  Time time;
  if(is_open_){
    int samples_per_word = 32 / nr_of_bitstreams;
    double subsec = (double)(current_header.frame_nr *  SIZE_MK5B_FRAME * samples_per_word) / sample_rate;
    if(subsec > 1)
      frame_nr_valid = false;
    if(frame_nr_valid)
      time.set_time(current_jday, current_header.seconds() + subsec);
    else
      time.set_time_usec(current_jday, current_header.microseconds());
  }
  return time;
}

bool Mark5b_reader::read_new_block(Data_frame &data) {
  std::vector<value_type> &buffer = data.buffer->data;
  if (buffer.size() != size_data_block()) {
    buffer.resize(size_data_block());
  }
  char *mark5b_block = (char *)&buffer[0];
  for (int i = 0 ; i < N_MK5B_BLOCKS_TO_READ ; i++) {
    if (i == 0) {
      int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                         sizeof(current_header),
                                                         (char *)&current_header );
      if(!check_header(current_header))
        return resync_header(data, 0); // Find next valid header in data file

      SFXC_ASSERT(current_header.check());
    } else {
      int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                         sizeof(current_header),
                                                         (char *)&tmp_header);
    }

    int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                       SIZE_MK5B_FRAME*SIZE_MK5B_WORD,
                                                       mark5b_block);

    mark5b_block += SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
  }

  if (data_reader_->eof()) return false;
  if (((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) != 0) && frame_nr_valid)
    return resync_header(data, 0);
  if(current_header.julian_day() != current_jday % 1000)
    current_jday++;
  data.start_time = get_current_time();
  // Check if there is a fill pattern in the data and if so, mark the data invalid
  find_fill_pattern(data);

  current_time_ = get_current_time();
  return true;
}

bool Mark5b_reader::eof() {
  return data_reader_->eof();
}

bool Mark5b_reader::Header::check() const {

  return (syncword == 0xABADDEED);
}

int64_t Mark5b_reader::Header::microseconds() const {
  int64_t sec = seconds();
  int64_t microseconds =
    (((((int64_t)subsec1)*10+subsec2)*10+subsec3)*10+subsec4)*100;
  return 1000000*sec + microseconds;
}

int64_t Mark5b_reader::Header::seconds() const {
  return
    (((((int32_t)sec1*10 + sec2)*10+sec3)*10+sec4)*10+sec5);
}

int Mark5b_reader::Header::julian_day() const {
  return ((((int32_t)day1)*10+day2)*10+day3);
}

Time Mark5b_reader::time_between_headers() {
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
  return time_between_headers_;
}

void Mark5b_reader::gen_crc16(){
  // Compute CRC 16 lookup table
  uint16_t crc, devisor = 0x8005; // 0xA001
  crc_table.resize(256);

  for(int i = 0 ; i < 256 ; i++){
    crc = (i << 8);
    for(int j = 8 ; j > 0 ; j--){
      if (crc & 0x8000){
        crc = (crc << 1) ^ devisor;
      }else{
        crc = (crc << 1);
      }
    }
    crc_table[i] = crc;
  }
}

bool Mark5b_reader::check_header(Header &header){
  // first check syncword
  if(header.syncword != 0xABADDEED)
    return false;

  // Check the CRC of the mark5b header
  unsigned int crc = 0;
  uint8_t *data = (uint8_t *) &header;
  for(int word = 2 ; word <= 3 ; word++){
    for(int i = 3 ; i >= 0 ; i--){
      crc ^= (data[word * 4 + i] << 8);
      crc = (crc << 8) ^ crc_table[(crc >> 8) & 0xff];
    }
  }
  return crc == 0;
}

void Mark5b_reader::set_parameters(const Input_node_parameters &param) {
  int tbr = param.track_bit_rate;
  SFXC_ASSERT((tbr % 1000000) == 0);
  SFXC_ASSERT((N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)%(tbr/1000000) == 0);
  time_between_headers_ = Time((double)N_MK5B_BLOCKS_TO_READ * SIZE_MK5B_FRAME / (tbr / 1000000));
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);

  sample_rate = param.sample_rate();
  // Find the number of bitstreams used
  nr_of_bitstreams = 32 / (param.channels[0].tracks.size() / param.channels[0].bits_per_sample);
  std::cout << "nbitstream = " << nr_of_bitstreams << ", tbr = " << tbr << "\n";
}

bool Mark5b_reader::resync_header(Data_frame &data, int try_) {
  const int max_read = RESYNC_MAX_DATA_FRAMES * size_data_block();
  const int frame_size = SIZE_MK5B_FRAME * SIZE_MK5B_WORD;
  int total_bytes_read = 0;
  std::cout << RANK_OF_NODE << " : Resync header, t = " << current_time_ << "\n";
  // Find the next header in the input stream
  char *buffer=(char *)&data.buffer->data[0];
  int buffer_size = data.buffer->data.size();
  int bytes_read, header_pos, write_pos = 0;
  bool syncword_found = false;

  // We first find the location of the next syncword
  while(total_bytes_read < max_read){
    bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), frame_size, buffer);
    total_bytes_read += bytes_read;
    if(bytes_read == 0){
      std::cout << "Couldn't find new sync word before EOF\n";
      return false;
    }
    for(header_pos = 0 ; header_pos < frame_size - 3 ; header_pos++){
      uint32_t *word = (uint32_t *)&buffer[header_pos];
      if(*word == 0xABADDEED){
        syncword_found = true;
        break;
      }
    }
    if(syncword_found){
      // Complete current frame
      char *header_buf = (char *)&current_header;
      int bytes_in_buffer = std::min((int)sizeof(current_header), frame_size - header_pos);
      int start = 0;
      memcpy((char *)&current_header, &buffer[header_pos], bytes_in_buffer);
      if(bytes_in_buffer < sizeof(current_header)){
        Data_reader_blocking::get_bytes_s(data_reader_.get(), sizeof(current_header) - bytes_in_buffer, &header_buf[bytes_in_buffer]);
        start = 0;
      }else{
        int frame_start = header_pos + sizeof(current_header);
        memmove(&buffer[0], &buffer[frame_start], frame_size - frame_start);
        start = frame_size - frame_start;
      }
      bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), frame_size - start, &buffer[start]);
      total_bytes_read += bytes_read;
      // because we assume that we always read N_MK5B_BLOCKS_TO_READ frames, we need to
      // find the first block for which frame_nr % N_MK5B_BLOCKS_TO_READ == 0
      if (current_header.frame_nr % N_MK5B_BLOCKS_TO_READ != 0){
        int nframes = N_MK5B_BLOCKS_TO_READ - (current_header.frame_nr % N_MK5B_BLOCKS_TO_READ);
        for(int j = 0; j < nframes ; j++){
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), sizeof(current_header), (char *)&tmp_header);
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), frame_size, buffer);
        }
      }
      // if headers are correct we are done
      if (check_header(current_header)){
        if(current_header.julian_day() != current_jday % 1000)
          current_jday++;
        data.start_time = get_current_time();
        find_fill_pattern(data);
        current_time_ = get_current_time();
        return true;
      }
    }
  }
  std::cout << "Couldn't find new sync word before EOF\n";
  return false;
}
