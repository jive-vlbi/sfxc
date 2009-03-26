#include "input_data_format_reader.h"

Input_data_format_reader::
Input_data_format_reader(boost::shared_ptr<Data_reader> data_reader)
  : data_reader_(data_reader) {
}

Input_data_format_reader::~Input_data_format_reader() {
}

bool Input_data_format_reader::eof() {
  return data_reader_->eof();
}
