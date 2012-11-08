#include "data_reader_blocking.h"
#include "vdif_reader.h"

VDIF_reader::VDIF_reader(boost::shared_ptr<Data_reader> data_reader,
			 Data_frame &data, Time ref_time)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    sample_rate(0)
{
  ref_jday = (int)ref_time.get_mjd();
}

VDIF_reader::~VDIF_reader() {}

bool 
VDIF_reader::open_input_stream(Data_frame &data) {
  if (!read_new_block(data)){
    if (eof())
      sfxc_abort("Could not find header before eof()");
    return false;
  }

  is_open_ = true;
  epoch_jday = current_header.jday_epoch();
  current_time_ = get_current_time();
  data.start_time = current_time_;
  uint start_sec = current_header.sec_from_epoch;
  uint epoch = current_header.ref_epoch;
  std::cout << RANK_OF_NODE << "Start of VDIF data at jday=" << epoch_jday + start_sec / (24 * 60 * 60)
            << ", seconds in epoch = " << start_sec << ", epoch=" << epoch 
            << ", t=" <<  current_time_ << std::endl;
  return true;
}

void 
VDIF_reader::print_header() {
  std::cerr << RANK_OF_NODE << "------------ full header ------------\n";
  std::cerr << current_header.sec_from_epoch<<" ; " << (int)current_header.legacy_mode << " ; "
            << (int)current_header.invalid<<"\n";
  std::cerr << current_header.dataframe_in_second << " ; " << (int)current_header.ref_epoch
            << " ; " << (int) current_header.unassiged<<"\n";
  std::cerr << current_header.dataframe_length << " ; " << (int) current_header.log2_nchan << " ; "
            << (int)current_header.version<<"\n";
  std::cerr << (int) current_header.station_id << " ; " << (int)current_header.thread_id << " ; "
            << (int) current_header.bits_per_sample << " ;" << (int) current_header.data_type<<"\n";
  std::cerr << current_header.user_data1<<" ; "<<(int)current_header.edv<<"\n";
  std::cerr << current_header.user_data2<<" ; "<<current_header.user_data3<<" ; "<< current_header.user_data4<<"\n";
  std::cerr << RANK_OF_NODE << "-------------------------------------\n";
}

Time
VDIF_reader::goto_time(Data_frame &data, Time time) {
  while (time > get_current_time()) {
    if (!read_new_block(data))
      break;
  }
  return get_current_time();
}

Time
VDIF_reader::get_current_time() {
  Time time;

  if (is_open_) {
    double seconds_since_reference = (double)current_header.sec_from_epoch - (ref_jday - epoch_jday) * 24 * 60 * 60;
    double subsec = 0;
    if (sample_rate > 0) {
      int samples_per_frame = 8 * current_header.data_size() / ((current_header.bits_per_sample + 1) * (1 << current_header.log2_nchan));
      subsec = (double)current_header.dataframe_in_second * samples_per_frame / sample_rate;
    }
    time.set_time(ref_jday, seconds_since_reference + subsec);
  }

  return time - offset;
}

bool
VDIF_reader::read_new_block(Data_frame &data) {
  std::vector<value_type> &buffer = data.buffer->data;

 restart:
  Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&current_header);
  if (data_reader_->eof())
    return false;

  int data_size = current_header.data_size();
  if (buffer.size() == 0) {
    memcpy(&first_header, &current_header, 16);
    buffer.resize(data_size);
  }

  if (current_header.dataframe_length != first_header.dataframe_length) {
    Data_reader_blocking::get_bytes_s(data_reader_.get(), first_header.dataframe_length * 8 - 16, NULL);
    goto restart;
  }

  SFXC_ASSERT(data_size == buffer.size());

  if (current_header.legacy_mode == 0) {
    char *header = (char *)&current_header;
    Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&header[16]);
    if (data_reader_->eof())
      return false;
  }

  Data_reader_blocking::get_bytes_s( data_reader_.get(), data_size, (char *)&buffer[0]);
  if (data_reader_->eof())
    return false;

  if (current_header.invalid > 0) {
    data.invalid.resize(1);
    data.invalid[0].invalid_begin = 0;
    data.invalid[0].nr_invalid = current_header.data_size();
    if (thread_map.count(current_header.thread_id) > 0)
      data.channel = thread_map[current_header.thread_id];
    else
      data.channel = 0;
  } else {
    if (thread_map.count(current_header.thread_id) == 0)
      goto restart;
    data.channel = thread_map[current_header.thread_id];
  }

  data.start_time = get_current_time();
  return true;
}

bool VDIF_reader::eof() {
  return data_reader_->eof();
}

int32_t VDIF_reader::Header::jday_epoch() const {
  int year = 2000 + ref_epoch / 2;
  int month = 1 + 6 * (ref_epoch % 2);
  return mjd(1, month, year);
}

Time VDIF_reader::time_between_headers() {
  int samples_per_frame = 8 * current_header.data_size() / ((current_header.bits_per_sample + 1) * (1 << current_header.log2_nchan));
  Time time_between_headers_(samples_per_frame / (sample_rate / 1000000));
  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
  return time_between_headers_;
}

void VDIF_reader::set_parameters(const Input_node_parameters &param) {
  sample_rate = param.sample_rate();
  SFXC_ASSERT(((int)sample_rate % 1000000) == 0);
  offset = param.offset;

  // Create a mapping from thread ID to channel number.
  thread_map.clear();
  if (param.n_tracks == 0) {
    for (size_t i = 0; i < param.channels.size(); i++)
      thread_map[param.channels[i].tracks[0]] = i;
  } else {
    thread_map[0] = 0;
  }
}
