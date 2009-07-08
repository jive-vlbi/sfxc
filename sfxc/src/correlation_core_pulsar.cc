#include "correlation_core_pulsar.h"
#include "output_header.h"
#include <utils.h>

Correlation_core_pulsar::Correlation_core_pulsar(){
 us_per_day=(int64_t)24*60*60*1000000;
#ifdef SFXC_WRITE_STATS
    SFXC_ASSERT_MSG(false,
                    "SFXC_WRITE_STATS currently doesn't work with pulsar binning.");
#endif // SFXC_WRITE_STATS
}

Correlation_core_pulsar::~Correlation_core_pulsar() {
}

void
Correlation_core_pulsar::set_parameters(const Correlation_parameters &parameters,
                                 int node_nr) {
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;

  size_t prev_size_of_fft = size_of_fft();
  correlation_parameters = parameters;

  create_baselines(parameters);

  double ms_per_day = 1000*24*60*60;
  start_mjd = parameters.start_time/ms_per_day + parameters.mjd;
  fft_duration = (((int64_t)n_channels())*1000000)/correlation_parameters.sample_rate;

  pulsar_params = parameters.pulsar_parameters;
  cur_pulsar = &pulsar_params->pulsars[std::string(&parameters.source[0])];
  if(accumulation_buffers.size()!=cur_pulsar->nbins)
    accumulation_buffers.resize(cur_pulsar->nbins);

  if(offsets.size()!=size_of_fft()/2+1)
    offsets.resize(size_of_fft()/2+1);
  if(bins.size() != size_of_fft()/2+1)
    bins.resize(size_of_fft()/2+1);

  // Find the appropiate polyco
  double diff = std::abs(cur_pulsar->polyco_params[0].tmid-start_mjd);
  int closest=0;
  for(int i=1; i<cur_pulsar->polyco_params.size(); i++){
    double new_diff=std::abs(cur_pulsar->polyco_params[i].tmid-start_mjd);
    if(new_diff<diff){
      diff=new_diff;
      closest = i;
    }
  }
  polyco = &cur_pulsar->polyco_params[closest];

  // Find the time offsets between frequency components
  int sb = correlation_parameters.sideband == 'L' ? -1 : 1;
  double base_freq = (parameters.channel_freq + sb*parameters.bandwidth*0.5)*1e-9; // [GHZ]
  double dfreq = parameters.bandwidth*1e-9/(n_channels()+1);
  // TODO check accuracy
  double ref_offset = 1/pow(polyco->obs_freq/1000,2);
  double freq = (polyco->n_coef>=2)?polyco->ref_freq+polyco->coef[1]/60:polyco->ref_freq;
  double period = 1000/freq;// in [ms]

  for(int i=0;i<size_of_fft()/2+1;i++){
    offsets[i] = 4.149*polyco->DM*(1/pow(base_freq+i*dfreq,2)-ref_offset)/period;
  }

  // Compute the phase at the start of the period
  double DT=(start_mjd+fft_duration*1./(2*us_per_day) - polyco->tmid)*1440;
  start_phase = (polyco->ref_phase-floor(polyco->ref_phase))+DT*60*polyco->ref_freq + polyco->coef[0];
  for (int i=1; i<polyco->coef.size(); i++){
    start_phase += polyco->coef[i]*pow(DT,i);
  }

}

void Correlation_core_pulsar::integration_initialise() {
  int size = (size_of_fft()/2+1);
  for(int bin=0;bin<accumulation_buffers.size();bin++){
    if (accumulation_buffers[bin].size() != baselines.size()) {
      accumulation_buffers[bin].resize(baselines.size());
      for (int j=0; j<accumulation_buffers[bin].size(); j++) {
        accumulation_buffers[bin][j].resize(size);
      }
    }
  }
  if (dedispersion_buffer.size() != baselines.size()) {
    dedispersion_buffer.resize(baselines.size());
    for (int j=0; j<dedispersion_buffer.size(); j++) {
      dedispersion_buffer[j].resize(size);
    }
  }

  for (int bin=0; bin<accumulation_buffers.size(); bin++) {
    for (int j=0; j<accumulation_buffers[bin].size(); j++) {
      SFXC_ASSERT(accumulation_buffers[bin][j].size() == size);
      memset(&accumulation_buffers[bin][j][0], size*sizeof(std::complex<FLOAT>), 0);
    }
  }
  for (int j=0; j<dedispersion_buffer.size(); j++) {
    SFXC_ASSERT(dedispersion_buffer[j].size() == size_of_fft()/2+1);
    memset(&dedispersion_buffer[j][0], size*sizeof(std::complex<FLOAT>), 0);
  }
}

void Correlation_core_pulsar::integration_step() {
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  if(check_input_elements){
    for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++) {
      if(!input_elements[i].valid()) 
        input_elements[i].set(&input_buffers[i]->front().data()[0],
                              input_buffers[i]->front().data().size());
    }
    check_input_elements=false;
  }
#ifndef DUMMY_CORRELATION
  // do the correlation
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    // Auto correlations
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first == stations.second);
    auto_correlate_baseline(/* in1 */ &input_elements[stations.first][0],
                            /* out */ &dedispersion_buffer[i][0]);
  }

  for (size_t i=number_input_streams_in_use(); i < baselines.size(); i++) {
    // Cross correlations
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first != stations.second);
    correlate_baseline(/* in1 */  &input_elements[stations.first][0],
                       /* in2 */  &input_elements[stations.second][0],
                       /* out */ &dedispersion_buffer[i][0]);
  }
  dedisperse_buffer();
#endif // DUMMY_CORRELATION

  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++){
    input_elements[i]++; // advance the iterator
    if(!input_elements[i].valid()){
      input_buffers[i]->pop();
      check_input_elements=true;
    }
  }
}

void Correlation_core_pulsar::integration_average() {
  std::vector<FLOAT> norms;
  norms.resize(n_stations());

  for (int bin=0; bin < accumulation_buffers.size(); bin++){
    memset(&norms[0], norms.size()*sizeof(FLOAT), 0);

    // Average the auto correlations
    for (size_t station=0; station < n_stations(); station++) {
      for (size_t i = 0; i < size_of_fft()/2+1; i++) {
        norms[station] += accumulation_buffers[bin][station][i].real();
      }
      norms[station] /= (size_of_fft()/2);

      for (size_t i = 0; i < size_of_fft()/2+1; i++) {
        // imaginary part should be zero!
        accumulation_buffers[bin][station][i] =
          accumulation_buffers[bin][station][i].real() / norms[station];
      }
    }

    // Average the cross correlations
    for (size_t station=n_stations(); station < baselines.size(); station++) {
      std::pair<size_t,size_t> &stations = baselines[station];
      FLOAT norm = sqrt(norms[stations.first]*norms[stations.second]);
      for (size_t i = 0 ; i < size_of_fft()/2+1; i++) {
        accumulation_buffers[bin][station][i] /= norm;
      }
    }
  }
}

void Correlation_core_pulsar::integration_write() {
  // Make sure that the input buffers are released
  // This is done by reference counting

  SFXC_ASSERT(writer != boost::shared_ptr<Data_writer>());
//  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());

  int polarisation = 1;
  if (correlation_parameters.polarisation == 'R') {
    polarisation =0;
  } else {
    SFXC_ASSERT(correlation_parameters.polarisation == 'L');
  }

  size_t n_stations = correlation_parameters.station_streams.size();
  std::vector<int> stream2station;

  {
    // initialise with -1
    stream2station.resize(input_buffers.size(), -1);

    for (size_t i=0;
         i < correlation_parameters.station_streams.size();
         i++) {
      size_t station_stream =
        correlation_parameters.station_streams[i].station_stream;
      SFXC_ASSERT(station_stream < stream2station.size());
      stream2station[station_stream] =
        correlation_parameters.station_streams[i].station_number;
    }
  }

  { // Writing the timeslice header
    Output_header_timeslice htimeslice;

    htimeslice.number_baselines = baselines.size();
    htimeslice.integration_slice =
      correlation_parameters.integration_nr + current_integration;
    htimeslice.number_uvw_coordinates = uvw_tables.size();

   // write the uvw coordinates
    Output_uvw_coordinates uvw[htimeslice.number_uvw_coordinates];
    // We evaluate in the middle of time slice (nb: the factor 1000 is for the conversion to microseconds)
    int64_t time;
    time=(int64_t)correlation_parameters.start_time*1000+(int64_t)correlation_parameters.integration_time*500;
    for (size_t station=0; station < uvw_tables.size(); station++){
     double u,v,w;
     uvw_tables[station].get_uvw(time, &u, &v, &w);
     uvw[station].station_nr=stream2station[station];
     uvw[station].u=u;
     uvw[station].v=v;
     uvw[station].w=w;
    }

    size_t nWrite = sizeof(htimeslice);
    writer->put_bytes(nWrite, (char *)&htimeslice);
    nWrite=sizeof(uvw);
    writer->put_bytes(nWrite, (char *)&uvw[0]);

    current_integration++;
  }

  accumulation_buffers_float.resize(size_of_fft()/2+1);

  Output_header_baseline hbaseline;
  for (int bin =0 ; bin < accumulation_buffers.size(); bin++){
    for (size_t i=0; i<baselines.size(); i++) {
      std::pair<size_t,size_t> &stations = baselines[i];

      for (size_t ii=0; ii<(size_of_fft()/2+1); ii++ ) {
        accumulation_buffers_float[ii] = accumulation_buffers[bin][i][ii];
      }

      hbaseline.weight = 0;       // The number of good samples

      // Station number in the vex-file
      SFXC_ASSERT(stations.first < n_stations);
      SFXC_ASSERT(stations.second < n_stations);
      hbaseline.station_nr1 = stream2station[stations.first];
      // Station number in the vex-file
      hbaseline.station_nr2 = stream2station[stations.second];

      // Polarisation for the first station
      SFXC_ASSERT((polarisation == 0) || (polarisation == 1)); // (RCP: 0, LCP: 1)
      hbaseline.polarisation1 = polarisation;
      hbaseline.polarisation2 = polarisation;
      if (correlation_parameters.cross_polarize) {
        if (stations.first >= n_stations/2)
          hbaseline.polarisation1 = 1-polarisation;
        if (stations.second >= n_stations/2)
          hbaseline.polarisation2 = 1-polarisation;
      }
      // Upper or lower sideband (LSB: 0, USB: 1)
      if (correlation_parameters.sideband=='U') {
        hbaseline.sideband = 1;
      } else {
        SFXC_ASSERT(correlation_parameters.sideband == 'L');
        hbaseline.sideband = 0;
      }
      // The number of the channel in the vex-file,
      hbaseline.frequency_nr = (unsigned char)correlation_parameters.channel_nr;
      // sorted increasingly
      // 1 byte left:
      hbaseline.empty = ' ';

      int nWrite = sizeof(hbaseline);
      writer->put_bytes(nWrite, (char *)&hbaseline);

      writer->put_bytes((size_of_fft()/2+1)*sizeof(std::complex<float>),
                        ((char*)&accumulation_buffers_float[0]));
    }
  }
}

void Correlation_core_pulsar::dedisperse_buffer() {
  double ref_phase = get_phase();
  double len=cur_pulsar->interval.stop-cur_pulsar->interval.start;
  // first compute the phase bins
  for(int j=0;j<size_of_fft()/2+1;j++){
    double phase = ref_phase+offsets[j];
    double dph = (phase-floor(phase)-cur_pulsar->interval.start);
    if((dph>=0)&&(dph<=len))
      bins[j] = (int)(dph*cur_pulsar->nbins/len);
    else
      bins[j]=-1;
  }

  // TODO check performance agains loop interchange
  for (int i=0; i < baselines.size(); i++) {
    for(int j=0;j<size_of_fft()/2+1;j++){
      int bin = bins[j];
      if(bin >= 0){
        accumulation_buffers[bin][i][j] += dedispersion_buffer[i][j];
      }
    }
  }
}
