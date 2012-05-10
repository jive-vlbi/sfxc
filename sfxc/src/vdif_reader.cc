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
  int byte_read = Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&current_header);
  if (current_header.legacy_mode == 0) {
    char *header = (char *)&current_header;
    Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&header[16]);
  }
  //  print_header();
  int data_size = current_header.data_size();
  if (buffer.size() != data_size) {
    buffer.resize(data_size);
  }

  if (current_header.invalid > 0) {
    data.invalid.resize(1);
    data.invalid[0].invalid_begin = 0;
    data.invalid[0].nr_invalid = current_header.data_size() * current_header.invalid;
  }
  data.channel = current_header.thread_id; // NB: we assume one-to-one mapping of thread id to channel

  byte_read = Data_reader_blocking::get_bytes_s( data_reader_.get(), data_size, (char *)&buffer[0]);

  if (data_reader_->eof()) return false;

  data.start_time = get_current_time();
  return true;
}

bool VDIF_reader::eof() {
  return data_reader_->eof();
}

int32_t VDIF_reader::Header::jday_epoch() const {
  int year = 2000 + ref_epoch / 2;
  int month = 1 + 6 * (ref_epoch % 1);
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
}
