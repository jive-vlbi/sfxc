#include <stdio.h>
#include <string>
#include <complex>
#include "sfxc_math.h"
#include "bandpass.h"
using namespace std;

bandpass::bandpass(){
  opened = false;
  freq_nr = 0;
}

void
bandpass::open_table(const std::string &name, int nchan_, bool phase_only){
  // Read bandpass table, downsampling the numbering of spectral points to nchan
  FILE *file = fopen(name.c_str(), "r");
  if (file == NULL)
    sfxc_abort(("Could not open BP table : " + name).c_str());
  nchan = nchan_;
  
  // Read header
  size_t nbytes;
  int32_t station;
  nbytes = fread(&nstation, 1, sizeof(nstation), file);
  nbytes = fread(&nif, 1, sizeof(nif), file);
  nbytes = fread(&npol, 1, sizeof(npol), file);
  nbytes = fread(&nchan_aips, 1, sizeof(nstation), file);
  cerr << "nstation =" << nstation << ", nif = "<< nif << ", npol = " << npol << ", nchan_aips " << nchan_aips << ", nchan = " << nchan <<"\n";
  // Read in table of frequencies
  frequencies.resize(nif);
  nbytes = fread(&frequencies[0], 1, nif * sizeof(double), file);
  // Read in the bandwidths
  bandwidths.resize(nif);
  nbytes = fread(&bandwidths[0], 1, nif * sizeof(double), file);
  // Read station number
  nbytes = fread(&station, 1, sizeof(station), file);
  if (nbytes != sizeof(station))
    throw string("premature end of bandpass table");

  // Used to store the complex band pass before we downsample to nchan spectral points
  vector<complex<float> > aips_bandpass;
  aips_bandpass.resize(nchan_aips * nif);
  
  bp_table.resize(nstation);
  while(nbytes == sizeof(station)){
    // Read bandpass and downsample to nchan spectral points per IF
    bp_table[station].resize(npol);
    for(int pol = 0; pol < npol; pol++){
      vector<complex<float> > &bp = bp_table[station][pol];
      bp.resize(nchan * nif);  
      nbytes = fread(&aips_bandpass[0], 1, nif*nchan_aips*sizeof(complex<float>), file);
      if (nbytes != nif*nchan_aips*sizeof(complex<float>))
        throw string("premature end of bandpass table");
      // Average down / up
      for(int i=0; i<nchan*nif; i++){
        int k = i * nchan_aips / nchan;
        if(std::abs(aips_bandpass[k]) < 0.01)
          bp[i] = 0;
        else{
          if(phase_only)
            bp[i] = std::abs(aips_bandpass[k]) / aips_bandpass[k];
          else
            bp[i] = float(1.) / aips_bandpass[k];
        }
      }
    }
    nbytes = fread(&station, 1, sizeof(station), file);
  }
  opened = true;
}

void 
bandpass::apply_bandpass(const Time t, complex<FLOAT> *band, int station, double freq, char sideband, int pol_nr, bool do_conjg){
  if (!opened)
    throw string("apply_bandpass called before opening bandpass table");
  // Get the index of the current_frequency
  int n = 0;
  int sb = (sideband == 'L') ? -1:0;
  double delta = bandwidths[freq_nr] / nchan_aips; // Lower side bands in AIPS are shifted by this amount
  while( (freq != frequencies[freq_nr] - sb*(bandwidths[freq_nr]-delta)) && (n < nif)){
    freq_nr = (freq_nr+1) % nif;
    delta = bandwidths[freq_nr] / nchan_aips;
    n++;
  }
  if (n == nif){
    cerr.precision(16);
    for(int i=0;i<nif;i++)
      cerr << "req = " << freq << ", ch[" << i << "]="<<frequencies[i] << ", sb * delta=" <<sb*delta<< "\n";
    cerr << "Requested frequency not in bandpass table\n";
    throw string("Requested frequency not in bandpass table");
  }
  vector<complex<float> > &bp = bp_table[station][pol_nr];
  if(sideband == 'L'){
    for(int i=0; i<nchan; i++){
      if (do_conjg)
        band[i+1] *= bp[(freq_nr+1)*nchan-i-1];
      else
        band[i+1] *= conj(bp[(freq_nr+1)*nchan-i-1]);
    }
  }else{
    SFXC_ASSERT(sideband == 'U');
    for(int i=0; i<nchan; i++){
      if(do_conjg)
        band[i] *= conj(bp[freq_nr*nchan+i]);
      else
        band[i] *= bp[freq_nr*nchan+i];
    }
    //SFXC_MUL_FC_I(&bp[freq_nr*nchan], band, nchan);
  }
}
