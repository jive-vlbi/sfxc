/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#include <assert.h>
#include <cmath>

#include "integer_delay_correction.h"

Integer_delay_correction::Integer_delay_correction() :
    output_buffer(Output_buffer_ptr(new Output_buffer(2))),
verbose(false) {}

Integer_delay_correction::~Integer_delay_correction() {}

void Integer_delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  assert(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
  input_reader = Input_reader_ptr(new Input_reader(input_buffer));
}

Integer_delay_correction::Output_buffer_ptr
Integer_delay_correction::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void Integer_delay_correction::set_delay_table(const Delay_table_akima &delay_table_) {
  delay_table_set = true;
  delay_table = delay_table_;
}

void Integer_delay_correction::set_parameters(const Correlation_parameters &param) {
  correlation_parameters = param;

  current_time = correlation_parameters.start_time*int64_t(1000);
  // We start with the data right away
  current_delay = 0;
}

int Integer_delay_correction::get_delay_in_samples(int64_t time) {
  assert(delay_table_set);

  return (int)std::floor(delay_table.delay(time) *
                         correlation_parameters.sample_rate +
                         .5);
}
int Integer_delay_correction::delta_time() {
  assert(correlation_parameters.number_channels*1000000 %
         correlation_parameters.sample_rate == 0);
  return correlation_parameters.number_channels*1000000 /
         correlation_parameters.sample_rate;
}

void Integer_delay_correction::do_task() {
  assert(has_work());
  Output_buffer_element &output = output_buffer->produce();
  if (output.size() != correlation_parameters.number_channels) {
    output.resize(correlation_parameters.number_channels);
  }

  // Compute integer delay for the sample in the middle
  int new_delay = get_delay_in_samples(current_time + delta_time()/2);

  if (new_delay == current_delay) {
    // Default case
    input_reader->get_bytes(correlation_parameters.number_channels,
                            (char *)output.buffer());
  } else if (new_delay == current_delay-1) {
    // Reuse last element
    output[0] = buffered_element;
    input_reader->get_bytes(correlation_parameters.number_channels-1,
                            (char *)&output[1]);
    current_delay = new_delay;
  } else if (new_delay == current_delay+1) {
    // Skip one element
    input_reader->get_bytes(1, (char *)output.buffer());
    input_reader->get_bytes(correlation_parameters.number_channels,
                            (char *)output.buffer());
    current_delay = new_delay;
  } else {
    // Initialising
    assert(new_delay < current_delay);
    // proceed one block
    current_delay -= correlation_parameters.number_channels;
    if (new_delay <= current_delay) {
      // Entire random block
    } else {
      int n_samples = new_delay-current_delay;
      assert(n_samples >= 0);
      input_reader->get_bytes
      (n_samples,
       (char *)&output[correlation_parameters.number_channels-n_samples]);
      current_delay = new_delay;
    }
  }
  buffered_element = output[correlation_parameters.number_channels-1];

  output_buffer->produced(correlation_parameters.number_channels);

  current_time += delta_time();
  assert(current_time >= 0);
}

bool Integer_delay_correction::has_work() {
  return !output_buffer->full();
}
