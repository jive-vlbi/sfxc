#include <limits>
#include <stdio.h>
#include "data_reader_blocking.h"
#include "mark5b_reader.h"

#define MAX_MARK5B_FRAME_NR 32768

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
              Data_frame &data, Time ref_date)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS), N(SIZE_MK5B_WORD), frame_nr(-1),
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
    if (eof())
      sfxc_abort("Could not find header before eof()");
    return false;
  }

  is_open_ = true;
  current_time_ = get_current_time();
  start_day_ = current_header.julian_day();
  start_time_ = current_time_;
  data.start_time = current_time_;
  std::cout << RANK_OF_NODE << "Start of Mark5b data at jday=" << start_day_
            << ", time = " << start_time_ << "\n";
  return true;
}

Time
Mark5b_reader::goto_time(Data_frame &data, Time time) {
  if (!data_reader_->is_seekable()){
    // Reading from a socket, read one frame at a time
    while (time > get_current_time()) {
      if (!read_new_block(data))
        break;
    }
  } else if (time > get_current_time()){
    // Search data with 1 second steps
    const size_t size_mk5b_block =
      (SIZE_MK5B_HEADER+SIZE_MK5B_FRAME)*SIZE_MK5B_WORD;

    // first search until we are within 1 sec from requested time
    const Time one_sec(1000000.);
    Time delta_time = time - get_current_time();
    while (delta_time >= one_sec){
      SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
      size_t n_blocks = one_sec / time_between_headers_;
      // Don't read the last header, to be able to check whether we are at the right time
      size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block;
      size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
      if (bytes_to_read != byte_read)
        return get_current_time();

      // Read last block:
      read_new_block(data);
      delta_time = time - get_current_time();
    }
    // Now read the last bit of data up to the requested time
    int n_blocks = (int)round(delta_time / time_between_headers_);
    if(n_blocks>0){
      // Don't read the last header, to be able to check whether we are at the right time
      size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block;
      size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
      if (bytes_to_read != byte_read)
        return get_current_time();
      
      read_new_block(data);
      if((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) != 0)
        return resync_header(data);
    }
  }
  return get_current_time();
}

Time Mark5b_reader::get_current_time() {
  Time time;
  if(is_open_){
    const double rate = (double)sample_rate * nr_of_bitstreams;
    const double subsec = (double)(frame_nr * 8 * SIZE_MK5B_FRAME * SIZE_MK5B_WORD) / rate;
    if((subsec > 1) && (frame_nr_valid)){
      std::cout << RANK_OF_NODE << " : Warning Mark5b header contains invalid frame_nr, switching to VLBA timestamp\n";
      frame_nr_valid = false;
    }
    if(frame_nr_valid)
      time.set_time(current_jday, current_header.seconds() + subsec);
    else
      time.set_time_usec(current_jday, current_header.microseconds());
    time -= offset;
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
                                                         sizeof(tmp_header),
                                                         (char *)&tmp_header );
      // Resync if frame is invalid or we didn't get the expected frame number
      if((!check_header(tmp_header)) || 
         (((tmp_header.frame_nr % N_MK5B_BLOCKS_TO_READ) != 0) && frame_nr_valid))
        return resync_header(data); // Find next valid header in data file
      previous_header = current_header;
      current_header = tmp_header;
      // in case of 4Gb/s data frame_nr wraps
      if ((nr_of_bitstreams == 64) && (current_header.seconds() == previous_header.seconds()) 
          && (frame_nr > current_header.frame_nr))
        frame_nr = current_header.frame_nr + MAX_MARK5B_FRAME_NR;
      else
        frame_nr = current_header.frame_nr;
    } else {
      int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                         sizeof(tmp_header),
                                                         (char *)&tmp_header);
    }
    int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                       SIZE_MK5B_FRAME*SIZE_MK5B_WORD,
                                                       mark5b_block);

    mark5b_block += SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
  }

  if (data_reader_->eof()) return false;
  if(current_header.julian_day() != current_jday % 1000)
    current_jday++;
  current_time_ = get_current_time();
  data.start_time = current_time_;
  // Check if there is a fill pattern in the data and if so, mark the data invalid
  data.invalid.resize(0);
  find_fill_pattern(data);

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

  if (header.crc == 0)
    return true;

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
  nr_of_bitstreams = param.n_tracks;
  // If there are 64 bitstreams then an input word is 8 bytes long, otherewise is is 4 bytes
  N = (nr_of_bitstreams <= 32)? 4: 8;
  int tbr = param.track_bit_rate;
  SFXC_ASSERT((tbr % 1000000) == 0);
  SFXC_ASSERT((N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)%(tbr/1000000) == 0);
  time_between_headers_ = Time((double)N_MK5B_BLOCKS_TO_READ * SIZE_MK5B_FRAME * SIZE_MK5B_WORD / (N * tbr / 1000000));
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
  sample_rate = param.sample_rate();

  offset = param.offset;
}

bool Mark5b_reader::resync_header(Data_frame &data) {
  const int max_read = RESYNC_MAX_DATA_FRAMES * size_data_block();
  const int header_size = SIZE_MK5B_HEADER * SIZE_MK5B_WORD;
  const int frame_size = SIZE_MK5B_FRAME * SIZE_MK5B_WORD;
  int total_bytes_read = 0;
  std::cout << RANK_OF_NODE << " : Resync header, t = " << current_time_ << "\n";
  // Find the next header in the input stream
  char *buffer=(char *)&data.buffer->data[0];
  int buffer_size = data.buffer->data.size();
  int bytes_read, header_pos, write_pos = 0;
  bool syncword_found = false;

  // We first find the location of the next syncword
  Data_reader_blocking::get_bytes_s(data_reader_.get(), header_size, buffer);
  while(total_bytes_read < max_read){
    bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), frame_size - header_size, &buffer[header_size]);
    total_bytes_read += bytes_read;
    if(bytes_read <= 0){
      std::cout << "Couldn't find new sync word before EOF, eof = " << eof() << "\n";
      return false;
    }
    for(header_pos = 0 ; header_pos < frame_size - header_size ; header_pos += SIZE_MK5B_WORD){
      uint32_t *word = (uint32_t *)&buffer[header_pos];
      if(*word == 0xABADDEED){
        syncword_found = true;
        break;
      }
    }
    if(!syncword_found){
      memcpy(buffer, &buffer[frame_size - header_size], header_size);
    }else{
      // Complete current frame
      char *header_buf = (char *)&tmp_header;
      int bytes_in_buffer = std::min((int)sizeof(tmp_header), frame_size - header_pos);
      int start = 0;
      memcpy((char *)&tmp_header, &buffer[header_pos], bytes_in_buffer);
      if(bytes_in_buffer < sizeof(tmp_header)){
        Data_reader_blocking::get_bytes_s(data_reader_.get(), sizeof(tmp_header) - bytes_in_buffer, &header_buf[bytes_in_buffer]);
      }else{
        int frame_start = header_pos + sizeof(tmp_header);
        memmove(&buffer[0], &buffer[frame_start], frame_size - frame_start);
        start = frame_size - frame_start;
      }
      bytes_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), frame_size - start, &buffer[start]);
      total_bytes_read += bytes_read;
      // because we assume that we always read N_MK5B_BLOCKS_TO_READ frames, we need to
      // find the first block for which frame_nr % N_MK5B_BLOCKS_TO_READ == 0
      if (tmp_header.frame_nr % N_MK5B_BLOCKS_TO_READ != 0){
        int nframes = N_MK5B_BLOCKS_TO_READ - (tmp_header.frame_nr % N_MK5B_BLOCKS_TO_READ);
        for(int j = 0; j < nframes ; j++){
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), sizeof(tmp_header), (char *)&tmp_header);
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), frame_size, buffer);
        }
      }
      if (check_header(tmp_header)){
        previous_header = current_header;
        current_header = tmp_header;
        // Now that we have found the first frame read the rest of the data
        for(int i = 1; i < N_MK5B_BLOCKS_TO_READ ; i++){
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), sizeof(tmp_header), (char *)&tmp_header);
          total_bytes_read += Data_reader_blocking::get_bytes_s( data_reader_.get(), frame_size, &buffer[i * frame_size]);
        }

        if(current_header.julian_day() != current_jday % 1000)
          current_jday++;
        // in case of 4Gb/s data frame_nr wraps
        if ((nr_of_bitstreams == 64) && (current_header.seconds() == previous_header.seconds()) 
            && (frame_nr > current_header.frame_nr))
          frame_nr = current_header.frame_nr + MAX_MARK5B_FRAME_NR;
        else
          frame_nr = current_header.frame_nr;
        current_time_ = get_current_time();
        data.start_time = current_time_;
        data.invalid.resize(0);
        find_fill_pattern(data);
        return true;
      }
    }
  }
  std::cout << "Couldn't find new sync word within search window\n";
  return false;
}
