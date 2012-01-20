#include "input_data_format_reader.h"

Input_data_format_reader::
Input_data_format_reader(boost::shared_ptr<Data_reader> data_reader)
  : data_reader_(data_reader), is_open_(false), offset(0.) {
}

Input_data_format_reader::~Input_data_format_reader() {
}

bool Input_data_format_reader::eof() {
  return data_reader_->eof();
}

void Input_data_format_reader::find_fill_pattern(Data_frame &data){
  int buffer_size = data.buffer->data.size() / 4; // number of 32 bit words in buffer
  uint32_t *buffer=(uint32_t *)&data.buffer->data[0];

  // See if there is already a bit of invalid data (assumed to start at byte 0)
  int end_previous_invalid = 0;
  if(data.invalid.size() > 0)
    end_previous_invalid = data.invalid[0].nr_invalid / 4;

  // If the first word contains fill pattern search for the end
  int start = end_previous_invalid;
  int end = start; 
  while(start < buffer_size -1){
    // find the start of the fill pattern
    if(buffer[start] != MARK5_FILLPATTERN){
      int step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - start - 1);
      while(step > 0){
        if(buffer[start + step] != MARK5_FILLPATTERN){
          start += step;
          if(start + step >= buffer_size)
            step = buffer_size - start - 1;
        }else{
          step /= 2;
        }
      }
      if (start < buffer_size -1)
        start += 1;
    }
    // Find the end of the fill pattern
    end = start;
    int step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - start - 1);
    while(step > 0){
      if(buffer[end + step] == MARK5_FILLPATTERN){
        end += step;
        if(end + step >= buffer_size)
          step = buffer_size - end - 1;
      }else{
        step /= 2;
      }
    }
    int old_size = data.invalid.size();
    data.invalid.resize(old_size + 1);
    data.invalid[old_size].invalid_begin = start* 4;
    data.invalid[old_size].nr_invalid = (end - start + 1) * 4; // nr_invalid is in bytes
    if (RANK_OF_NODE == -1){
      std::cout << RANK_OF_NODE << " : " << (end - start + 1) << " words of fill pattern found1\n";
    }
    start = end + 1;
  }
}
