/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

// Output_node_controller is defined in Output_node.h:
#include "output_node.h"

#include "utils.h"
#include "data_writer_file.h"
#include "data_reader_tcp.h"
#include "tcp_connection.h"
#include "correlator_time.h"

#include <iostream>


Output_node_controller::Output_node_controller(Output_node &node)
  : Controller(node), node(node)
{
}

Controller::Process_event_status
Output_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_OUTPUT_NODE_GLOBAL_HEADER: {
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      SFXC_ASSERT(size == sizeof(Output_header_global));
      struct Output_header_global global_header;
      MPI_Recv((char *)&global_header, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.write_global_header(global_header);

      struct Output_header_phasecal phasecal_header;
      phasecal_header.header_size = sizeof(phasecal_header);
      memcpy(&phasecal_header.experiment, global_header.experiment,
	     sizeof(phasecal_header.experiment));
      phasecal_header.output_format_version =
	global_header.output_format_version;
      phasecal_header.correlator_version = global_header.correlator_version;
      memcpy(&phasecal_header.correlator_branch,
	     global_header.correlator_branch,
	     sizeof(phasecal_header.correlator_branch));
      phasecal_header.job_nr = global_header.job_nr;
      phasecal_header.subjob_nr = global_header.subjob_nr;
      phasecal_file.write((char *)&phasecal_header, sizeof(phasecal_header));

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      int32_t weight[4]; // stream, slicenr, size (in bytes), n_bins
      MPI_Recv(&weight, 4, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      SFXC_ASSERT(status.MPI_SOURCE == status2.MPI_SOURCE);
      SFXC_ASSERT(status.MPI_TAG == status2.MPI_TAG);

      // Create an output buffer:
      node.set_weight_of_input_stream(weight[0], weight[1], weight[2], weight[3]);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_NODE_CORRELATION_READY: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      int32_t nr_of_time_slices;
      MPI_Recv(&nr_of_time_slices, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      SFXC_ASSERT(nr_of_time_slices >= 0);
      node.set_number_of_time_slices(nr_of_time_slices);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_NODE_SET_PHASECAL_FILE: {
      int len;
      MPI_Get_elements(&status, MPI_CHAR, &len);
      SFXC_ASSERT(len > 0);

      char filename[len];
      MPI_Recv(&filename, len, MPI_CHAR, status.MPI_SOURCE,
	       status.MPI_TAG, MPI_COMM_WORLD, &status2);
      SFXC_ASSERT(filename[len - 1] == 0);
      SFXC_ASSERT(strncmp(filename, "file://", 7) == 0);
      phasecal_file.open(filename + 7, std::ios::out | std::ios::trunc | std::ios::binary);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_NODE_WRITE_PHASECAL: {
      int len;
      MPI_Get_elements(&status, MPI_CHAR, &len);

      char msg[len];
      MPI_Recv(&msg, len, MPI_CHAR, status.MPI_SOURCE,
	       status.MPI_TAG, MPI_COMM_WORLD, &status2);

      int pos = 0;
      int32_t station_number, channel_number;
      MPI_Unpack(msg, len, &pos, &station_number, 1, MPI_INT32, MPI_COMM_WORLD);
      MPI_Unpack(msg, len, &pos, &channel_number, 1, MPI_INT32, MPI_COMM_WORLD);

      Time start_time;
      uint64_t start_time_ticks;
      MPI_Unpack(msg, len, &pos, &start_time_ticks, 1, MPI_INT64, MPI_COMM_WORLD);
      start_time.set_clock_ticks(start_time_ticks);
      uint32_t start_secs = (start_time.get_mjd() - 40587) * 86400;

      Time integration_time;
      uint64_t integration_time_ticks;
      MPI_Unpack(msg, len, &pos, &integration_time_ticks, 1, MPI_INT64, MPI_COMM_WORLD);
      integration_time.set_clock_ticks(integration_time_ticks);
      uint32_t integration_time_secs = integration_time.get_time();

      uint32_t num_samples;
      MPI_Unpack(msg, len, &pos, &num_samples, 1, MPI_INT32, MPI_COMM_WORLD);
      std::vector<int32_t> samples;
      samples.resize(num_samples);
      MPI_Unpack(msg, len, &pos, &samples[0], num_samples, MPI_INT32, MPI_COMM_WORLD);

      SFXC_ASSERT(phasecal_file.is_open());
      phasecal_file.write((char *)&station_number, sizeof(station_number));
      phasecal_file.write((char *)&channel_number, sizeof(channel_number));
      phasecal_file.write((char *)&start_secs, sizeof(start_secs));
      phasecal_file.write((char *)&integration_time_secs, sizeof(integration_time_secs));
      phasecal_file.write((char *)&num_samples, sizeof(num_samples));
      phasecal_file.write((char *)&samples[0], num_samples * sizeof(samples[0]));

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
