#include "data_reader_blocking.h"
#include "vdif_reader.h"

VDIF_reader::VDIF_reader(boost::shared_ptr<Data_reader> data_reader,
			 Data_frame &data, Time ref_time)
  : Input_data_format_reader(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
    sample_rate(0), first_header_seen(false)
{
  ref_jday = (int)ref_time.get_mjd();
}

VDIF_reader::~VDIF_reader() {}

bool 
VDIF_reader::open_input_stream(Data_frame &data) {
  if (!read_new_block(data)){
    return false;
  }

  is_open_ = true;
  current_time_ = get_current_time();
  data.start_time = current_time_;
  int32_t epoch_jday = current_header.jday_epoch();
  uint32_t start_sec = current_header.sec_from_epoch;
  uint32_t epoch = current_header.ref_epoch;
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
    double seconds_since_reference = (double)current_header.sec_from_epoch - (ref_jday - current_header.jday_epoch()) * 24 * 60 * 60;
    double subsec = 0;
    if (sample_rate > 0) {
      int samples_per_frame = 8 * first_header.data_size() / ((first_header.bits_per_sample + 1) * (1 << first_header.log2_nchan));
      subsec = (double)current_header.dataframe_in_second * samples_per_frame / sample_rate;
    }
    time.set_time(ref_jday, seconds_since_reference + subsec);
  }

  return time - offset;
}

bool
VDIF_reader::read_new_block(Data_frame &data) {
  std::vector<value_type> &buffer = data.buffer->data;
  const int max_restarts = 256;
  int restarts = 0;

 restart:
  if (!first_header_seen) {
    Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&current_header);
    if (data_reader_->eof())
      return false;

    memcpy(&first_header, &current_header, 16);
    first_header_seen = true;

    if (first_header.legacy_mode == 0) {
      // FIXME : If first header has fill pattern this will fail
      // We should use the information that vex2 provides
      char *header = (char *)&current_header;
      Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&header[16]);
      if (data_reader_->eof())
	return false;
    }
  } else {
    if (first_header.legacy_mode == 0) {
      Data_reader_blocking::get_bytes_s(data_reader_.get(), 32, (char *)&current_header);
    } else {
      Data_reader_blocking::get_bytes_s(data_reader_.get(), 16, (char *)&current_header);
    }
    if (data_reader_->eof())
      return false;
  }

  int data_size = first_header.data_size();
  if (buffer.size() == 0)
    buffer.resize(N_VDIF_FRAMES_PER_BLOCK * data_size);

  if (((uint32_t *)&current_header)[0] == 0x11223344 ||
      ((uint32_t *)&current_header)[1] == 0x11223344 ||
      ((uint32_t *)&current_header)[2] == 0x11223344 ||
      ((uint32_t *)&current_header)[3] == 0x11223344 ||
      (current_header.dataframe_in_second % N_VDIF_FRAMES_PER_BLOCK) != 0) {
    Data_reader_blocking::get_bytes_s(data_reader_.get(), data_size, NULL);
    if (++restarts > max_restarts)
      return false;
    goto restart;
  }

  Data_reader_blocking::get_bytes_s( data_reader_.get(), data_size, (char *)&buffer[0]);
  if (data_reader_->eof())
    return false;

  if (current_header.invalid > 0) {
    data.invalid.resize(1);
    data.invalid[0].invalid_begin = 0;
    data.invalid[0].nr_invalid = data_size;
    if (thread_map.count(current_header.thread_id) > 0)
      data.channel = thread_map[current_header.thread_id];
    else
      data.channel = 0;
  } else {
    if (thread_map.count(current_header.thread_id) == 0) {
      // If this is the only thread we obtain its thread_id from the data
      if(thread_map.size() == 0) {
        thread_map[current_header.thread_id] = 0;
      } else {
	if (++restarts > max_restarts)
	  return false;
        goto restart;
      }
    }
    data.channel = thread_map[current_header.thread_id];
  }

  for (int i = 1; i < N_VDIF_FRAMES_PER_BLOCK; i++) {
    if (first_header.legacy_mode == 0) {
      Data_reader_blocking::get_bytes_s( data_reader_.get(), 32, NULL);
    } else {
      Data_reader_blocking::get_bytes_s( data_reader_.get(), 16, NULL);
    }
    if (data_reader_->eof())
      return false;

    Data_reader_blocking::get_bytes_s( data_reader_.get(), data_size, (char *)&buffer[i * data_size]);
    if (data_reader_->eof())
      return false;
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

void VDIF_reader::set_parameters(const Input_node_parameters &param) {
  sample_rate = param.sample_rate();
  SFXC_ASSERT(((int)sample_rate % 1000000) == 0);
  offset = param.offset;

  // Create a mapping from thread ID to channel number.
  // If n_tracks > 0 then the data contains a single VDIF thread,
  // the thread_id of this thread comes from the actual data
  thread_map.clear();
  if (param.n_tracks == 0) {
    for (size_t i = 0; i < param.channels.size(); i++)
      thread_map[param.channels[i].tracks[0]] = i;
    time_between_headers_ = Time(param.frame_size * 8.e6 / (sample_rate * param.bits_per_sample()));
    bits_per_complete_sample = param.bits_per_sample();
  } else {
    time_between_headers_ = Time(N_VDIF_FRAMES_PER_BLOCK * param.frame_size * 8.e6 / (sample_rate * param.n_tracks));
    bits_per_complete_sample = param.n_tracks;
  }

  SFXC_ASSERT(time_between_headers_.get_time_usec() > 0);
}
