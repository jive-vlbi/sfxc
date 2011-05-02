#include "input_data_format_reader.h"

Input_data_format_reader::
Input_data_format_reader(boost::shared_ptr<Data_reader> data_reader)
  : data_reader_(data_reader), is_open_(false) {
}

Input_data_format_reader::~Input_data_format_reader() {
}

bool Input_data_format_reader::eof() {
  return data_reader_->eof();
}

void Input_data_format_reader::find_fill_pattern(Data_frame &data){
  int buffer_size = data.buffer->data.size() / 4; // number of 32 bit words in buffer
  int step_size = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size) / 2;
  uint32_t *buffer=(uint32_t *)&data.buffer->data[0];

  // See if there is already a bit of invalid data (assumed to start at byte 0)
  int end_previous_invalid = 0;
  if(data.invalid.size() > 0)
    end_previous_invalid = data.invalid[0].nr_invalid / 4;

  // If the first word contains fill pattern search for the end
  int start = end_previous_invalid;
  int end = start;
  int step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size) / 2;
  if(buffer[start] == MARK5_FILLPATTERN){
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
    data.invalid[old_size].invalid_begin = start;
    data.invalid[old_size].nr_invalid = (end - start + 1) * 4; // nr_invalid is in bytes
    std::cout << RANK_OF_NODE << " : " << (end - start + 1) << " words of fill pattern found1\n";
    start = end + 1;
  }

  // find the start of the first block of fill pattern
  step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - start) / 2;
  while(step > 0){
    if(buffer[start + step] != MARK5_FILLPATTERN){
      start += step;
      if(start + step >= buffer_size)
        step = buffer_size - start - 1;
    }else{
      step /= 2;
    }
  }

  // Now find all other blocks of invalid data which are garanteed to be 
  // spaced in multples of MARK5_FILLPATTERN_NWORDS
  while(start < buffer_size -1){
    // Find the end of the invalid block
    int end = start;
    step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - end -1);
    while((buffer[end + step] == MARK5_FILLPATTERN) && (step > 0)){
      end += step;
      step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - end -1);
    }

    // insert the invalid block into the list of invalid data
    int old_size = data.invalid.size();
    data.invalid.resize(old_size + 1);
    data.invalid[old_size].invalid_begin = start;
    data.invalid[old_size].nr_invalid = (end - start + 1) * 4; // nr_invalid is in bytes
    std::cout << RANK_OF_NODE << " : " << (end - start + 1 ) << " words of fill pattern found2\n";

    // Find the start of the next invalid block
    start = end + 1;
    do{
      step = std::min(MARK5_FILLPATTERN_NWORDS, buffer_size - start -1);
      start += step;
    }while((step > 0) && (buffer[start] != MARK5_FILLPATTERN));
  }
}
