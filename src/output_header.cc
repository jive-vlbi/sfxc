#include "output_header.h"

std::ostream &
operator<<(std::ostream &out,
           const Output_header_global &global_header) {
  out << "{ \"header_size\": " << global_header.header_size
  << "," << std::endl
  << "  \"experiment\": \"" << global_header.experiment << "\""
  << "," << std::endl
  << "  \"start_year\": " << global_header.start_year
  << "," << std::endl
  << "  \"start_day\": " << global_header.start_day
  << "," << std::endl
  << "  \"start_time\": " << global_header.start_time
  << "," << std::endl
  << "  \"number_channels\": " << global_header.number_channels
  << "," << std::endl
  << "  \"integration_time\": " << (int)global_header.integration_time
  << "," << std::endl
  << "  \"polarisation_type\": " << (int)global_header.polarisation_type
  << " }"
  << std::endl;

  return out;
}

std::ostream &
operator<<(std::ostream &out,
           const Output_header_timeslice &timeslice_header) {
  out << "{ \"integration_slice\": " << timeslice_header.integration_slice
  << "," << std::endl
  << "  \"number_baselines\": " << timeslice_header.number_baselines
  << "," << std::endl
  << "  \"number_uvw_coordinates\": " << timeslice_header.number_uvw_coordinates
  << " }"
  << std::endl;

  return out;
}

std::ostream &
operator<<(std::ostream &out,
           const Output_uvw_coordinates &uvw_header) {
  out << "{ \"station_nr\": " << uvw_header.station_nr
  << "," << std::endl
  << "  \"u\": " << uvw_header.u
  << "," << std::endl
  << "  \"v\": " << uvw_header.v
  << "," << std::endl
  << "  \"w\": " << uvw_header.w
  << " }"
  << std::endl;

  return out;
}

std::ostream &
operator<<(std::ostream &out,
           const Output_header_baseline &baseline_header) {
  out << "{ \"weight\": " << (int)baseline_header.weight
  << "," << std::endl
  << "  \"station_nr1\": " << (int)baseline_header.station_nr1
  << "," << std::endl
  << "  \"station_nr2\": " << (int)baseline_header.station_nr2
  << "," << std::endl
  << "  \"polarisation1\": " << (int)baseline_header.polarisation1
  << "," << std::endl
  << "  \"polarisation2\": " << (int)baseline_header.polarisation2
  << "," << std::endl
  << "  \"sideband\": " << (int)baseline_header.sideband
  << "," << std::endl
  << "  \"frequency_nr\": " << (int)baseline_header.frequency_nr
  << " }"
  << std::endl;

  return out;
}
