#include "data_reader_blocking.h"
#include "mark5b_reader.h"
#include <stdio.h>

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
              Data_frame &data, Time ref_date)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    time_between_headers_(0.), sample_rate(0)
{
  //us_per_day=(int64_t)24*60*60*1000000;
  // Initialize the crc16 lookup table
  gen_crc16();

  current_jday = ref_date.get_mjd();
  if(!read_new_block(data)){
    if(!resync_header(data))
      sfxc_abort("Couldn't find valid mark5b header");
  }

  start_day_ = current_header.julian_day();
  start_time_ = current_time_;
  std::cout << RANK_OF_NODE << "Start of Mark5b data at jday=" << start_day_
            << ", time = " << start_time_ << "\n";
}


Mark5b_reader::~Mark5b_reader() {}

Time
Mark5b_reader::goto_time(Data_frame &data, Time us_time) {
  SFXC_ASSERT(current_header.check());
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
  current_time_ = get_current_time();

  if (us_time <= current_time_) return current_time_;

  const size_t size_mk5b_block_header =
    (SIZE_MK5B_HEADER+SIZE_MK5B_FRAME)*SIZE_MK5B_WORD;

  // first skip through the file in 1 second steps.
  const Time one_sec(1000000.);
  Time delta_time = us_time - current_time_;
  while (delta_time >= one_sec){
    SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
    int n_blocks = (int)(one_sec/time_between_headers_) - current_header.frame_nr/N_MK5B_BLOCKS_TO_READ;

    // Don't read the last header, to be able to check whether we are at the right time
    size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block_header;
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
    if (bytes_to_read != byte_read)
      return current_time_;

    // Read last block:
    read_new_block(data);
    delta_time = us_time - current_time_;
  }
  // Now read the last bit of data up to the requested time
  int n_blocks = (int)round(delta_time/time_between_headers_) -
                  current_header.frame_nr/N_MK5B_BLOCKS_TO_READ;
  if(n_blocks>0){
    // Don't read the last header, to be able to check whether we are at the right time
    size_t bytes_to_read = (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block_header;
    size_t byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );
    if (bytes_to_read != byte_read)
      return current_time_;

    read_new_block(data);
    SFXC_ASSERT((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) == 0);
  }
  current_time_ = get_current_time();

  return current_time_;
}

Time Mark5b_reader::get_current_time() {
  Time time;
  if(sample_rate > 0){
    int samples_per_word = 32 / nr_of_bitstreams;
    double subsec = (double)(current_header.frame_nr *  SIZE_MK5B_FRAME * samples_per_word) / sample_rate;
    time.set_time(current_jday, current_header.seconds() + subsec);
  } else {
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
        return resync_header(data); // Find next valid header in data file

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
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
  if(current_header.julian_day() != current_jday % 1000)
    current_jday++;
  data.start_time = get_current_time();
  // Check if there is a fill pattern in the data and if so, mark the data invalid
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

std::vector< std::vector<int> >
Mark5b_reader::get_tracks(const Input_node_parameters &input_node_param,
                          Data_frame & /* data */) {
  std::vector< std::vector<int> > result;
  result.resize(input_node_param.channels.size());
  for (size_t i=0; i<input_node_param.channels.size(); i++) {
    int bps = input_node_param.channels[i].bits_per_sample();
    int fo  = input_node_param.channels[i].sign_tracks.size();
    result[i].resize(bps * fo);
    for (size_t j=0; j<input_node_param.channels[i].sign_tracks.size(); j++) {
      if (bps == 1) {
        result[i][j] = input_node_param.channels[i].sign_tracks[j];
      } else {
        SFXC_ASSERT(bps == 2);
        result[i][2*j] = input_node_param.channels[i].sign_tracks[j];
        result[i][2*j+1] = input_node_param.channels[i].magn_tracks[j];
      }
    }
  }
  return result;
}

void Mark5b_reader::set_parameters(const Input_node_parameters &param) {
  int tbr = param.track_bit_rate;
  SFXC_ASSERT((tbr % 1000000) == 0);
  SFXC_ASSERT((N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)%(tbr/1000000) == 0);
  time_between_headers_ = Time((double)N_MK5B_BLOCKS_TO_READ * SIZE_MK5B_FRAME / (tbr / 1000000));
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);

  sample_rate = param.sample_rate();
  // Find the number of bitstreams used
  nr_of_bitstreams = 32 / param.channels[0].sign_tracks.size();
  sample_rate = param.sample_rate();
  // Find the number of bitstreams used
  nr_of_bitstreams = 32 / param.channels[0].sign_tracks.size();

}

bool Mark5b_reader::resync_header(Data_frame &data) {
  // Find the next header in the input stream
  char *buffer=(char *)&data.buffer->data[0];
  int buffer_size = data.buffer->data.size();
  int bytes_read, header_pos = 0, write_pos = 0;
  bool syncword_found = false;

  // We first find the location of the next syncword
  bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), 
                                                  SIZE_MK5B_FRAME * SIZE_MK5B_WORD,
                                                  buffer);
  while (!syncword_found){
    if(bytes_read == 0){
      std::cout << "Couldn't find new sync word before EOF\n";
      return false;
    }
    for(header_pos = 0 ; header_pos < SIZE_MK5B_FRAME * SIZE_MK5B_WORD - 3 ; header_pos++){
      uint32_t *word = (uint32_t *)&buffer[header_pos];
      if(*word == 0xABADDEED){
        syncword_found = true;
        break;
      }
    }
    if(!syncword_found){
      memcpy(&buffer[0], &buffer[SIZE_MK5B_FRAME * SIZE_MK5B_WORD -3], 3); 
      bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), 
                                                      SIZE_MK5B_FRAME * SIZE_MK5B_WORD - 3,
                                                      &buffer[3]);
    }
  }

  // because we assume that we always read N_MK5B_BLOCKS_TO_READ frames, we need to
  // find the first block for which frame_nr % N_MK5B_BLOCKS_TO_READ == 0
  memcpy((char *)&current_header, &buffer[header_pos], sizeof(current_header));
  if (current_header.frame_nr % N_MK5B_BLOCKS_TO_READ != 0){
    int nframes = N_MK5B_BLOCKS_TO_READ - (current_header.frame_nr % N_MK5B_BLOCKS_TO_READ);
    bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                    header_pos + sizeof(current_header),
                                                    buffer);
    for(int j = 0; j < nframes - 1 ; j++){
      bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                      sizeof(current_header),
                                                      (char *)&tmp_header);
      bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                      SIZE_MK5B_FRAME * SIZE_MK5B_WORD,
                                                      buffer);
    }
    bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                    sizeof(current_header),
                                                    (char *)&current_header);
  }else{
    int nbytes = SIZE_MK5B_FRAME * SIZE_MK5B_WORD - header_pos - sizeof(current_header);
    memcpy(&buffer[0], &buffer[header_pos + sizeof(current_header)], nbytes);
    write_pos = nbytes;
  }
  if (!check_header(current_header))
    return resync_header(data);

  // Not that the first header is read, we read the rest of the data
  bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                  SIZE_MK5B_FRAME * SIZE_MK5B_WORD - write_pos,
                                                  &buffer[write_pos]);
  for (int j = 1; j < N_MK5B_BLOCKS_TO_READ; j++) {
    bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                    sizeof(current_header),
                                                    (char *)&tmp_header);
    bytes_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
                                                    SIZE_MK5B_FRAME * SIZE_MK5B_WORD,
                                                    &buffer[j * SIZE_MK5B_FRAME * SIZE_MK5B_WORD]);
  }

  if(bytes_read == 0){ 
    std::cout << "Couldn't find new sync word before EOF\n";
    return false;
  }

  if(current_header.julian_day() != current_jday % 1000)
    current_jday++;

  data.start_time = get_current_time();
  find_fill_pattern(data);

  return true;
}