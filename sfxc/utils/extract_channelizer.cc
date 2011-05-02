/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 * This file extract from a control file/vex file the different
 * channelizer that are needed for a sfxc experiment.
 */
#include <cassert>
#include <iostream>

#undef USE_MPI
#include "exception_common.h"
#include "control_parameters.h"
#include "log_writer_cout.h"
#include "mark5a_reader.h"
#include "data_reader_file.h"
#include "data_reader_factory.h"
#include "channel_extractor_dynamic.h"
#include "correlator_time.h"

std::string dstdir="./";

int main(int argc, char** argv)
{
  try
    {

      if (argc < 4)
        {
          std::cout << "Usage: " << argv[0] << " <ctrl-file> <vex-file> <output-dir>"  << std::endl;
          exit(-1);
        }

      const char * ctrl_file = (const char*)argv[1];
      const char * vex_file = (const char*)argv[2];
      dstdir = argv[3];


      Control_parameters control_parameters;
      Log_writer_cout log_writer(10);
      control_parameters.initialise(ctrl_file, vex_file, log_writer);

      Time start = control_parameters.get_start_time();
      Time stop = control_parameters.get_stop_time();

      // Get a list of all scan names
      int begin_scan = control_parameters.scan(start);
      int current_scan = control_parameters.scan(start);
      int end_scan = control_parameters.scan(stop);

      SFXC_ASSERT(current_scan >= 0);
      SFXC_ASSERT((size_t)current_scan < control_parameters.number_scans());

      std::cout << "number of scans:" << end_scan-current_scan+1 << std::endl;

      for (;current_scan<=end_scan;current_scan++)
        {
          std::string scan_name=control_parameters.scan(current_scan);
          std::cout << "SCAN: " << scan_name << std::endl;
          std::cout << "number of station:" << control_parameters.number_stations_in_scan(scan_name) << std::endl;

          const std::string &mode_name = control_parameters.get_vex().get_mode(scan_name);
          std::cout << "MODE: " << mode_name << std::endl;
          for (size_t station=0;station<control_parameters.number_stations(); station++)
            {
              std::string station_name = control_parameters.station(station);
              std::cout << "TRANSPORT TYPE:" << control_parameters.transport_type(station_name) << std::endl;

              Input_node_parameters input_node_param = control_parameters.get_input_node_parameters(mode_name, station_name);


              Mark5a_reader::Data_frame data;

              std::string urlsrc = control_parameters.data_sources(station_name)[current_scan-begin_scan];
              boost::shared_ptr<Data_reader> reader= boost::shared_ptr<Data_reader>( Data_reader_factory::get_reader(urlsrc) );
              boost::shared_ptr<Mark5a_reader> m_reader =
                boost::shared_ptr<Mark5a_reader>( new Mark5a_reader(reader, Time(0)) );
              while((!m_reader->open_input_stream(data)) && (!m_reader->eof()))
                ;
              int n_subbands = input_node_param.channels.size();
              int bits_per_sample = input_node_param.bits_per_sample();
              int fan_out    = bits_per_sample * input_node_param.subsamples_per_sample();
              int samples_per_block = SIZE_MK5A_FRAME;
              m_reader->get_current_time();
              std::cout << "Channelizer !" << std::endl;
              Channel_extractor_dynamic channelizer(dstdir, true);
              channelizer.initialise(m_reader->get_tracks(input_node_param, 
                                                          data) ,
                                     m_reader->N,
                                     samples_per_block, bits_per_sample);

              std::cout << "The channelizer that is in use is so: " << channelizer.name() << std::endl;
            }
        }
    }
  catch (Exception& ex)
    {
      std::cout << ex << std::endl;
    }
}

