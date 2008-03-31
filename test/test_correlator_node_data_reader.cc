#include "correlator_node_data_reader_tasklet.h"
#include "correlator_node_types.h"
#include "data_reader_file.h"

char *infile = "file://data/input.txt";
char *outfile = "file://output.txt";

int main(int argc, char *argv[]) {
  typedef Correlator_node_types::Bit_sample_queue Bit_sample_queue;
  typedef Bit_sample_queue::value_type            Bit_sample_queue_elem;

  boost::shared_ptr<Data_reader> reader(new Data_reader_file(infile));
  Correlator_node_data_reader_tasklet corr_data_reader;

  corr_data_reader.connect_to(reader);
  int n_ffts_to_read  = 100;
  int bits_per_sample = 2;
  int number_channels = 1024;
  corr_data_reader.set_parameters(n_ffts_to_read,
                                  bits_per_sample,
                                  number_channels);

  while (corr_data_reader.has_work()) {
    corr_data_reader.do_task();

    if (!corr_data_reader.get_output_buffer()->empty()) {
      corr_data_reader.get_output_buffer()->front().release();
      corr_data_reader.get_output_buffer()->pop();
    }
  }

}
