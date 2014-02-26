#include <stdio.h>
#include <string>
#include <complex>
#include "sfxc_math.h"
#include "aips_cal.h"
using namespace std;
#define SPEED_OF_LIGHT (299792458.)
#define MINIMUM_WEIGHT (0.001)

aips_cal::aips_cal(){
  opened = false;
  freq_nr = 0;
}

void
aips_cal::open_table(const std::string &name, int nchan_){
  // Read calibration data from aips CL table
  FILE *file = fopen(name.c_str(), "r");
  if (file == NULL)
    sfxc_abort(("Could not open CL table : " + name).c_str());
  
  nchan = nchan_;
 
  // Read header
  size_t nbytes;
  int32_t station;
  nbytes = fread(&start_mjd, 1, sizeof(start_mjd), file);
  nbytes = fread(&nchan_aips, 1, sizeof(nchan_aips), file);
  nbytes = fread(&nstation, 1, sizeof(nstation), file);
  nbytes = fread(&npol, 1, sizeof(npol), file);
  nbytes = fread(&nif, 1, sizeof(nif), file);
  // Read in table of frequencies
  frequencies.resize(nif);
  nbytes = fread(&frequencies[0], 1, nif * sizeof(double), file);
  // Read in the bandwidths
  bandwidths.resize(nif);
  nbytes = fread(&bandwidths[0], 1, nif * sizeof(double), file);
  if (nbytes != nif * sizeof(double))
    throw string("premature end of cl table");

  // Compute the number of entries
  size_t row_size = nstation * npol * nif * 6 * sizeof(double) ;
  size_t start_pos = ftell(file);
  fseek(file, 0, SEEK_END);
  size_t n_entries = (ftell(file) - start_pos + 1) / row_size;
  fseek(file, start_pos, SEEK_SET);

  // Create data structures
  times.resize(n_entries);
  time_interval.resize(n_entries);
  cl_table.resize(nstation);
  for(int i=0; i < nstation; i++){
    cl_table[i].delays.resize(n_entries * nif * npol);
    cl_table[i].rates.resize(n_entries * nif * npol);
    cl_table[i].gains.resize(n_entries * nif * npol);
    cl_table[i].weights.resize(n_entries * nif * npol);
    cl_table[i].disp_delays.resize(n_entries * nif * npol);
  }

  // Read table
  int row = 0;
  int64_t time, interval;
  vector<double> buffer(nstation * nif * npol * 6);
  nbytes = fread(&time, 1, sizeof(int64_t), file);
  nbytes = fread(&interval, 1, sizeof(int64_t), file);
  nbytes = fread(&buffer[0], 1, buffer.size() * sizeof(double), file); 
  while(nbytes == buffer.size() * sizeof(double)){
    times[row] = Time(start_mjd, time/1000000.);
    time_interval[row] = Time(interval);
    if(RANK_OF_NODE == -10)
      std::cout << "row = " <<  row << ", time = " << times[row] << ", interval = " << time_interval[row] <<"\n";

    for(int station = 0; station < nstation; station++){
      int i = row * npol * nif;
      int j = station * npol * nif * 6;
      for(int pol = 0; pol < npol; pol++){
        for(int if_nr = 0; if_nr < nif; if_nr++){
          cl_table[station].delays[i] = buffer[j++];
          cl_table[station].rates[i] = buffer[j++];
          double norm = sqrt(buffer[j]*buffer[j] + buffer[j+1]*buffer[j+1]);//FIXME Experimental
          if (abs(norm) < 1)
            norm = 1;
          cl_table[station].gains[i] = complex<double>(buffer[j], buffer[j+1])/norm;
          // FIXME Even more experimental
          cl_table[station].gains[i] = complex<double>(buffer[j], buffer[j+1])/(norm*norm);
          j += 2;
          cl_table[station].weights[i] = buffer[j++];
          cl_table[station].disp_delays[i] = buffer[j++];
          if ((RANK_OF_NODE == -10) && (if_nr == 0) && (pol ==0)) cout << "t=" << times[row] <<", station << "<< station<< ", weight = " << cl_table[station].weights[i]
                                                                      << ", disp_delay = " << cl_table[station].disp_delays[i] << "\n";
          i += 1;
        }
      }
    }
    if(RANK_OF_NODE == -10) cout << RANK_OF_NODE << " : t = "<< times[row] << ", interval  = " << time_interval[row]
                                << ", time_orig = " << time << ", mjd =" << start_mjd<< "\n";
    row += 1;
    nbytes = fread(&time, 1, sizeof(int64_t), file);
    nbytes = fread(&interval, 1, sizeof(int64_t), file);
    nbytes = fread(&buffer[0], 1, buffer.size() * sizeof(double), file); 
  }
  init();
  opened = true;
}

void
aips_cal::init(){
  // Init vector containing the current row indices, these are not necessarily the same for all subbands because of weights
  current_row.resize(nstation);  
  next_row.resize(nstation);
  int nrows = times.size();
  for(int i=0;i<nstation;i++){
    current_row[i].resize(0);
    current_row[i].resize(npol*nif, -1);
    next_row[i].resize(npol*nif);
    for(int pol=0;pol<npol;pol++){
      for(int if_nr = 0;if_nr<nif;if_nr++){
        const int idx = pol*nif + if_nr;
        int row = find_next_row(i, pol, if_nr);
        current_row[i][idx] = row;
        row = find_next_row(i, pol, if_nr);
        next_row[i][idx] = row;
      }
    }
  }
}

int 
aips_cal::find_next_row(int station, int pol_nr, int if_nr){
  // Get the next CL table row whoose weight is above the weight cutoff
  const int nrows = times.size();
  const int idx = pol_nr * nif + if_nr;
  int row = current_row[station][idx] + 1;
  if((row<nrows)&&(station ==2)&&(RANK_OF_NODE==-10)) std::cerr << RANK_OF_NODE << "<CHECK> "<< row << " :  weight next = " << cl_table[station].weights[row*npol*nif+idx] << ", tnext = " << times[row] << "\n";
  while((row < nrows-1) && (cl_table[station].weights[row*npol*nif+idx] < MINIMUM_WEIGHT)){
    row++;
    if((row<nrows)&&(station ==2)&&(RANK_OF_NODE==-10)) std::cerr << RANK_OF_NODE << "<CHECK> "<< row << " :  weight next = " << cl_table[station].weights[row*npol*nif+idx] << ", tnext = " << times[row] << "\n";
  }
  return (row < nrows)? row : nrows-1;
}

void 
aips_cal::apply_calibration(const Time t, complex<FLOAT> *band, int station, double freq, char sideband, int pol_nr, bool do_conjg){
  // NB : do_conjg has the default argument : false
  if (!opened)
    throw string("apply_calibration called before opening table");
  // Get the index of the current_frequency
  int n = 0;
  int sb = (sideband == 'L') ? -1:0;
  double delta = bandwidths[freq_nr] / nchan_aips; // Lower side bands in AIPS are shifted by this amount
  while( (freq != frequencies[freq_nr] - sb*(bandwidths[freq_nr]-delta)) && (n < nif)){
    freq_nr = (freq_nr+1) % nif;
    delta = bandwidths[freq_nr] / nchan_aips;
    n++;
  }
  //cerr << RANK_OF_NODE << " : apply, freq_nr = " << freq_nr << "\n";
  if (n == nif){
    cerr.precision(16);
    for(int i=0;i<nif;i++)
      cerr << "req = " << freq << ", ch[" << i << "]="<<frequencies[i] << ", sb * delta=" <<sb*delta<< "\n";
    cerr << "Requested frequency not in aips_cal table\n";
    throw string("Requested frequency not in aips_cal table");
  }
 
  // Move to the correct time
  int idx = pol_nr * nif + freq_nr;
  if((station == 2)&&(RANK_OF_NODE==-10)) std::cerr << RANK_OF_NODE << " : idx = " << idx << ", pol_nr = " << pol_nr << " / " << npol << ", freq = " << freq_nr << " / " << nif << "; size = " << ", station = " << station << ", nstation = " << current_row.size() << "\n";
  int current = current_row[station][idx];
  int next = next_row[station][idx];
  if((station ==2)&&(RANK_OF_NODE==-10)) std::cerr << RANK_OF_NODE << ": move to correct time, next = " << next << ", current = " << current << ", t= " << t << ", tnext = " << times[next] << "\n";
  if(t > times[next]){
    current_row[station][idx] = next;
    next = find_next_row(station, pol_nr, freq_nr);
    while((next < times.size()-1) && (t > times[next])){
      current_row[station][idx] = next;
      next = find_next_row(station, pol_nr, freq_nr);
      if((station ==2)&&(RANK_OF_NODE==-10)) std::cerr << RANK_OF_NODE << "move2 to correct time, next = " << next << ", current = " << current_row[station][idx]
                << "t= " << t << ", tnext = " << times[next] << "\n";
    }
    current = current_row[station][idx];
    next_row[station][idx] = next;
  }
  if((t < times[current]) || (t>times[next])){
    if((station ==2)&&(RANK_OF_NODE==-10)) cerr << RANK_OF_NODE << " : t = " << t<< ", freq = " << freq_nr << ", pol = "<< pol_nr << " is not in cl table for station " << station << "\n";
    return; // no valid calibration data
  }

  // Apply the calibation FIXME : This can be optimized
  double clint = (times[next]-times[current]).get_time();
  if (clint == 0){
    cout << "duplicate !\n";
    return;
  }
  double dt = (t - times[current]).get_time();
  double df = bandwidths[freq_nr] / nchan; 
  int index1 = current*nif*npol + pol_nr*nif + freq_nr;
  int index2 = next*nif*npol + pol_nr*nif + freq_nr;
  // Get the delay (linearly intepolatated)
  double delay1 = cl_table[station].delays[index1];
  double delay2 = cl_table[station].delays[index2];
  double w1 = (clint-dt)/clint, w2= dt/clint;
  double delay = w1*delay1 + w2*delay2;
  // The rate is a bit more complex, first compute complex gain and then interpolate the complex values
  double ph_rate1 = 2.*M_PI*cl_table[station].rates[index1]*frequencies[freq_nr]*dt;
  double ph_rate2 = 2.*M_PI*cl_table[station].rates[index2]*frequencies[freq_nr]*(dt-clint);
  double crate_real = cos(ph_rate1)*w1 + cos(ph_rate2)*w2;
  double crate_imag = sin(ph_rate1)*w1 + sin(ph_rate2)*w2;
  double ph_rate = atan2(crate_imag, crate_real);
  // The amplitude from the complex gains are obtained through linear interpolation, the phases by
  // interpolating the complex gains and taking the argument of the result.
  complex<double> gain1 = cl_table[station].gains[index1];
  complex<double> gain2 = cl_table[station].gains[index2];
  complex<double> gain = w1*gain1 + w2*gain2;
  double phase = atan2(imag(gain), real(gain));
  double amplitude = abs(gain1) * w1 + abs(gain2) * w2;
  // Get the dispersive delay
  double ddelay1 = cl_table[station].disp_delays[index1];
  double ddelay2 = cl_table[station].disp_delays[index2];
  double ddelay = w1*ddelay1 + w2*ddelay2;
  if(RANK_OF_NODE == -19) cout << "t = " << t <<", delay1 =" << delay1 << ", gain = " << gain1 <<", df = " << df << ", f = " 
                              << frequencies[freq_nr] <<", nchan = " << nchan << "freq = " << freq << ", sideband = " << sideband
                              << ", nchan_aips = " << nchan_aips << ", bw = " << bandwidths[freq_nr] << ", station = " << station << "\n";
  // AIPS uses channel center with possibly a different number of channels
  double phase_offset = 2 * M_PI * bandwidths[freq_nr] * delay / (2* nchan_aips); 
  phase_offset = 0.; // FIXME Remove this!
  const double conj = do_conjg ? -1 : 1;
  if(sideband == 'L'){
    for(int i=0; i<nchan; i++){
      double phi = 2 * M_PI * (i * df * delay) + ph_rate + phase - phase_offset;
      double frac = SPEED_OF_LIGHT * SPEED_OF_LIGHT / (freq - (nchan-1-i) * df);
      phi += 2 * M_PI * frac * ddelay;
      band[nchan - 1 - i] *= amplitude*complex<double>(cos(phi), conj*sin(phi));
    }
  }else{
    SFXC_ASSERT(sideband == 'U');
    for(int i=0; i<nchan+1; i++){
      double phi = -2 * M_PI * (i * df * delay) - ph_rate - phase + phase_offset;
      double frac = SPEED_OF_LIGHT * SPEED_OF_LIGHT / (freq + i * df);
      phi += -2 * M_PI * frac * ddelay;
      band[i] *= amplitude*complex<double>(cos(phi), conj*sin(phi));
    }
    //SFXC_MUL_FC_I(&bp[freq_nr*nchan], band, nchan);
  }
}
