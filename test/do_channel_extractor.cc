#include "mark4_reader.h"
#include "data_reader_file.h"

char    *filename = "file:///data4/sfxc/ftp/2007_june/f07m2/mk4/f07m2_cm_no0001.m5a";
int64_t start_time = 58078000000;

const FLOAT sample_value_ms[] = {-7, 2, -2, 7};

template <class Type>
void print_tracks(Type /* No name */,
                  boost::shared_ptr<Data_reader> reader,
                  char *buffer,
                  Control_parameters &parameters,
                  std::ostream &out) {
  Type block[SIZE_MK4_FRAME];
  Mark4_reader<Type> mark4_reader(reader, buffer, &block[0]);


  std::string station = parameters.station(0);
  std::string scan = 
    parameters.scan(parameters.scan(parameters.get_start_time()));
  std::string mode = 
    parameters.get_vex().get_mode(scan);
  
  std::vector<int> tracks;
  Input_node_parameters input_parameters =
    parameters.get_input_node_parameters(mode, station);
  tracks = mark4_reader.get_tracks(input_parameters, block)[0];

  std::cerr << "Tracks: ";
  for (size_t i=0; i<tracks.size(); i++) {
    std::cerr << tracks[i] << " ";
  }
  std::cerr << std::endl;

  int fan_out = tracks.size()/2;
  std::cerr << "fan_out: " << fan_out << std::endl;
  for (int block_nr=0; block_nr<100; block_nr++) {
    for (size_t i=0; i<SIZE_MK4_FRAME; i++) {
      for (int j = 0; j<fan_out; j++) {
        std::cout << ((block[i] >> tracks[j*2]) & 1) 
                  << ((block[i] >> tracks[j*2+1]) & 1)
                  << std::endl;
        int sample = 
          ((block[i] >> tracks[j*2]) & 1)*2 +
          ((block[i] >> tracks[j*2+1]) & 1);
        out << sample_value_ms[sample] << std::endl;
      }
    }
    mark4_reader.read_new_block(block);
  }
}


int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "Usage: " << argv[0] << " <ctrl-file> <vex-file> <out-file>" 
              << std::endl;
    exit(-1);
  }

  Control_parameters parameters(argv[1], argv[2], std::cerr);

  std::string station = parameters.station(0);
  std::cerr << "Station: " << station << std::endl;
  std::string filename = parameters.data_sources(station)[0];
  std::cerr << "Filename: " << filename << std::endl;

  boost::shared_ptr<Data_reader> data_reader(new Data_reader_file(filename));
  char buffer[SIZE_MK4_FRAME];
  int n_tracks = find_start_of_header(data_reader, buffer);

  
  std::ofstream out(argv[3]);
  switch (n_tracks) {
  case 1: {
    print_tracks((int8_t)0, data_reader, buffer, parameters, out);
    break;
  }
  case 2: {
    print_tracks((int16_t)0, data_reader, buffer, parameters, out);
    break;
  }
  case 4: {
    print_tracks((int32_t)0, data_reader, buffer, parameters, out);
    break;
  }
  case 8: {
    print_tracks((int64_t)0, data_reader, buffer, parameters, out);
    break;
  }
  default: {
    assert(false);
  }
  }
}
