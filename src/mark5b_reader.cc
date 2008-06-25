#include "mark5b_reader.h"

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
              Data_frame &data)
    : data_reader_(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    time_between_headers_(0) {
  SFXC_ASSERT(sizeof(current_header) == SIZE_MK5B_HEADER*SIZE_MK5B_WORD);
  data_reader_->get_bytes(sizeof(current_header), (char *)&current_header);
  SFXC_ASSERT_MSG(current_header.check(),
                  "Couldn't find the mark5b header in the mark5b file");

  int blocksize = N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
  data.mark5_data.resize(blocksize);
  char *buffer = (char *)&data.mark5_data[0];

  data_reader_->get_bytes(SIZE_MK5B_FRAME * SIZE_MK5B_WORD, buffer);

  for (int i=1; i<N_MK5B_BLOCKS_TO_READ; i++) {
    buffer += SIZE_MK5B_FRAME * SIZE_MK5B_WORD;

    data_reader_->get_bytes(sizeof(current_header), (char *)&tmp_header);
    SFXC_ASSERT(tmp_header.check());
    
    data_reader_->get_bytes(SIZE_MK5B_FRAME * SIZE_MK5B_WORD,
                            (char *)buffer);
  }

  start_day_ = current_header.julian_day();
  start_time_ = current_header.microseconds();
}


Mark5b_reader::~Mark5b_reader() {}

int64_t
Mark5b_reader::goto_time(unsigned char *mark5b_block, int64_t us_time) {
  SFXC_ASSERT(current_header.check());
  SFXC_ASSERT(time_between_headers_ > 0);
  int64_t current_time_ = current_header.microseconds();

  if (us_time <= current_time_) return current_time_;

  const int64_t delta_time = us_time-current_header.seconds()*1000000;

  SFXC_ASSERT(delta_time % time_between_headers_ == 0);
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
  int n_blocks = 
    delta_time/time_between_headers_ - 
    current_header.frame_nr/N_MK5B_BLOCKS_TO_READ;

  const int size_mk5b_block_header =
    (SIZE_MK5B_HEADER+SIZE_MK5B_FRAME)*SIZE_MK5B_WORD;

  // Don't read the last header, to be able to check whether we are at the
  // right time
  int bytes_to_read = 
    (n_blocks-1)*N_MK5B_BLOCKS_TO_READ*size_mk5b_block_header;

  int bytes_read = data_reader_->get_bytes(bytes_to_read, NULL);
  SFXC_ASSERT(bytes_to_read == bytes_read);

  // Read last block:
  read_new_block(mark5b_block);

  SFXC_ASSERT((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) == 0);
  current_time_ = current_header.microseconds();
  SFXC_ASSERT(us_time == current_time_);
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);

  return current_time_;
}

int64_t Mark5b_reader::get_current_time() {
  return current_header.microseconds();
}


bool Mark5b_reader::read_new_block(unsigned char *mark5b_block) {
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);

  for (int i=0; i<N_MK5B_BLOCKS_TO_READ; i++) {
    if (i==0) {
      data_reader_->get_bytes(sizeof(current_header), 
                              (char *)&current_header);
      SFXC_ASSERT(current_header.check());
    } else {
      data_reader_->get_bytes(sizeof(current_header), 
                              (char *)&tmp_header);
      SFXC_ASSERT(tmp_header.check());
    }

    data_reader_->get_bytes(SIZE_MK5B_FRAME*SIZE_MK5B_WORD,
                            (char *)mark5b_block);
    mark5b_block += SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
  }

  return current_header.check();
}

bool Mark5b_reader::eof() {
  return data_reader_->eof();
}

bool Mark5b_reader::Header::check() const {
  if (syncword != 0xABADDEED) 
    return false;

  return true;
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

void Mark5b_reader::set_track_bit_rate(int tbr) {
  SFXC_ASSERT((tbr % 1000000) == 0);
  SFXC_ASSERT((N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)%(tbr/1000000) == 0);
  time_between_headers_ = 
    (N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)/(tbr/1000000);
  SFXC_ASSERT(time_between_headers_ > 0);
}

int Mark5b_reader::time_between_headers() {
  SFXC_ASSERT(time_between_headers_ > 0);
  return time_between_headers_;
}
