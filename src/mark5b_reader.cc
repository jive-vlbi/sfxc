#include "data_reader_blocking.h"
#include "mark5b_reader.h"

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
              Data_frame &data)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    time_between_headers_(0)
{
  // initially use start_date as reference
  read_new_block(data);
  ref_jday = current_header.julian_day(); 
  SFXC_ASSERT(current_header.check());

  start_day_ = current_header.julian_day();
  start_time_ = current_header.microseconds();
  DEBUG_MSG("Start of Mark5b data at jday=" << start_day_
            << ", time = " << start_time_);
  us_per_day=(int64_t)24*60*60*1000000;
}


Mark5b_reader::~Mark5b_reader() {}

int64_t
Mark5b_reader::goto_time(Data_frame &data, int64_t us_time) {
  SFXC_ASSERT(current_header.check());
  SFXC_ASSERT(time_between_headers_ > 0);
  int64_t current_time_ = correct_raw_time(current_header.microseconds());

  if (us_time <= current_time_) return current_time_;

  const int64_t delta_time = us_time-correct_raw_time((int64_t)current_header.seconds()*1000000);

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

  /// int bytes_read = data_reader_->get_bytes(bytes_to_read, NULL);
  int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), bytes_to_read, NULL );

  SFXC_ASSERT(bytes_to_read == byte_read);

  // Read last block:
  read_new_block(data);

  SFXC_ASSERT((current_header.frame_nr % N_MK5B_BLOCKS_TO_READ) == 0);
  current_time_ = correct_raw_time(current_header.microseconds());
  SFXC_ASSERT(us_time == current_time_);
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);

  return current_time_;
}

int64_t Mark5b_reader::correct_raw_time(int64_t raw_time){
  // Convert time read from input stream to time relative to midnight on the reference day
  int cur_jday = current_header.julian_day();
  int64_t ret_val;
  if(cur_jday >= ref_jday)
    ret_val = raw_time + (cur_jday-ref_jday)*us_per_day;
  else
    ret_val = raw_time + (1000+cur_jday-ref_jday)*us_per_day;

  return ret_val;
}

int64_t Mark5b_reader::get_current_time() {

  return correct_raw_time(current_header.microseconds());
}

bool Mark5b_reader::read_new_block(Data_frame &data) {
  if (data.buffer.size() != size_data_block()) {
    data.buffer.resize(size_data_block());
  }
  data.invalid_bytes_begin = 0;
  data.nr_invalid_bytes = 0;
  char * mark5b_block = (char *)&data.buffer[0];
  for (int i=0; i<N_MK5B_BLOCKS_TO_READ; i++) {
    if (i==0) {
			int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
																												sizeof(current_header),
																												(char *)&current_header );

      SFXC_ASSERT(current_header.check());
    } else {
   		int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
   																											sizeof(current_header),
																												(char *)&tmp_header);
      SFXC_ASSERT(tmp_header.check());
    }

    int byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(),
   																											SIZE_MK5B_FRAME*SIZE_MK5B_WORD,
																												mark5b_block);

    mark5b_block += SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
  }

  if (data_reader_->eof()) return false;
  SFXC_ASSERT(current_header.frame_nr % N_MK5B_BLOCKS_TO_READ == 0);
  data.start_time = correct_raw_time(current_header.microseconds());
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

int Mark5b_reader::time_between_headers() {
  SFXC_ASSERT(time_between_headers_ > 0);
  return time_between_headers_;
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
  time_between_headers_ =
    (N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME)/(tbr/1000000);
  SFXC_ASSERT(time_between_headers_ > 0);
  ref_jday = (mjd(1,1,param.start_year) + param.start_day -1 )%1000;
  DEBUG_MSG("Ref_jday=" << ref_jday);
}
