#include <iostream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "utils.h"
#ifdef __linux
#include <endian.h>
#else
#include <sys/endian.h>
#endif
#include "create_psrfits.h"
using namespace std;
FILE *uit;
bool flag = false;
#define POL_R  0
#define POL_L  1
#define POL_RL 2
#define POL_I  3


SFXCdata::SFXCdata(const char *infilename, const char *vexname){
  vex.open(vexname);
  infile = fopen(infilename, "r");
  if (!infile)
    throw "Could not open input file";

  // Obtain correlation from input file
  get_parameters();

  current_int = -1; // current integration
}

void 
SFXCdata::get_parameters(){
  // Read the global header, the header size of the current data file might
  // be different than that of the current global_header_size
  int32_t data_header_size = 0;
  size_t nbytes = fread(&data_header_size, 1, sizeof(int32_t), infile);
  if (nbytes != sizeof(int32_t))
    throw "Error : Premature EOF in input file";
  fseek(infile, 0, 0);
  int32_t global_header_size = std::min((size_t) data_header_size, 
                                        sizeof(Output_header_global));
  nbytes = fread(&gheader, 1, global_header_size, infile);
  if ((nbytes != global_header_size) || (nbytes==0))
    throw "Error : Premature EOF in input file";
  fseek(infile, data_header_size, 0);
  // Get the meta information from vex file
  string exper_id = vex.get_root_node()["GLOBAL"]["EXPER"]->to_string();
  pi_name = vex.get_root_node()["EXPER"][exper_id]["PI_name"]->to_string();  
  exper_name = vex.get_root_node()["EXPER"][exper_id]["exper_name"]->to_string(); 
  cout << exper_id << pi_name << exper_name << "\n"; 
  // Get the source information
  Vex::Date scan_start(gheader.start_year, gheader.start_day, gheader.start_time);
  string scan_name = vex.get_scan_name(scan_start);
  cout << "scan_name = " << scan_name << "\n";
  src_name = vex.get_root_node()["SCHED"][scan_name]["source"]->to_string();
  src_coords(src_name);

  // Get the number of polarizations
  npol = 1;
  if ((gheader.polarisation_type == gheader.LEFT_RIGHT_POLARISATION) ||
      (gheader.polarisation_type == gheader.LEFT_RIGHT_POLARISATION_WITH_CROSSES))
    npol = 2;

  // Get a list of all subbands which are in the vex file
  vector<subband> vex_subbands;
  get_subbands(vex_subbands);
  // Create a search structure, to easily map subband_nr to frequency
  vector<int> vex_subband_idx[2];
  double prev = -1;
  for(int i=0; i<vex_subbands.size(); i++){
    int size = vex_subband_idx[0].size();
    if(prev != vex_subbands[i].frequency){
      size += 1;
      prev = vex_subbands[i].frequency;
      vex_subband_idx[0].resize(size);
      vex_subband_idx[1].resize(size);
    }
    int sb = vex_subbands[i].sideband; // 0 = LSB, 1 = USB
    vex_subband_idx[sb][size-1]=i;
  }

  // Get the actually correlated subbands from the first integration
  set<int> subband_set;
  Output_header_timeslice htimeslice;
  nbytes = fread(&htimeslice, 1, sizeof(htimeslice), infile);
  nsamples = htimeslice.number_baselines; 
  sizeof_integration = 0;
  while ((htimeslice.integration_slice == 0) && (nbytes == sizeof(htimeslice))){
    Output_header_baseline hbaseline;
    Output_uvw_coordinates huvw;
    Output_header_bitstatistics hstatistics;
    const int data_size = (gheader.number_channels+1) * sizeof(float); 
    unsigned char buffer[data_size];

    sizeof_integration += nbytes;
    for(int i=0; i< htimeslice.number_uvw_coordinates; i++)
      sizeof_integration += fread(&huvw, 1, sizeof(huvw), infile);
    for(int i=0; i< htimeslice.number_statistics; i++)
      sizeof_integration += fread(&hstatistics, 1, sizeof(hstatistics), infile);
    for(int i=0; i<htimeslice.number_baselines; i++){
      sizeof_integration += fread(&hbaseline, 1, sizeof(hbaseline), infile);
      sizeof_integration += fread(buffer, 1, data_size, infile);
      //cout << "band = " << (int)hbaseline.frequency_nr << ", sb=" << (int)hbaseline.sideband << "\n";
      int sb = hbaseline.sideband;
      int freq_nr = hbaseline.frequency_nr;
      subband_set.insert(vex_subband_idx[sb][freq_nr]);
    }
    nbytes = fread(&htimeslice, 1, sizeof(htimeslice), infile);
  }
  cout << "slice = " << htimeslice.integration_slice << ", bytes =" << nbytes << ", asked = " << sizeof(htimeslice) << "\n";
  // Store the subbands which were found in the data in a sorted vector
  // At the same time construct a search structure to relate frequency_nr to subband
  subbands_index[0].resize(0); // make sure all elements are initialized
  subbands_index[0].resize(vex_subbands.size(), -1);
  subbands_index[1].resize(0); // make sure all elements are initialized
  subbands_index[1].resize(vex_subbands.size(), -1);
  int subband_nr = -1;
  int i=0;
  prev = -1;
  for (set<int>::iterator it = subband_set.begin(); it !=subband_set.end(); it++){
    subband &current_subband = vex_subbands[*it];
    while(i<=*it){
      if (vex_subbands[i].frequency != prev){
        prev = vex_subbands[i].frequency;
        subband_nr++;
      }
      i++;
    }
    cout << "push : freq_nr="<< *it << ", f = " << current_subband.frequency << ", SB = " << current_subband.sideband<<"\n";
    subbands.push_back(current_subband);
    subbands_index[current_subband.sideband][subband_nr] = subbands.size()-1;
  }
  cout << "subbands_size = " << subbands.size() << "\n";
  fseek(infile, 0, SEEK_END);
  long size = ftell(infile);
  nsubint = size / sizeof_integration;
  fseek(infile, global_header_size, 0); // Reset file pointer
}

void 
SFXCdata::get_subbands(vector<subband> &vex_subbands){
  Vex::Date scan_start(gheader.start_year, gheader.start_day, gheader.start_time);
  string scan_name = vex.get_scan_name(scan_start);
  string mode = vex.get_mode(scan_name);
  // Get sorted list of all frequencies
  vector<double> bands;
  vex.get_frequencies(mode, bands);

  // create sorted list of subbands
  Vex::Node root_node = vex.get_root_node();
  Vex::Node::iterator mode_it = root_node["MODE"][mode];
  std::string freq_node = mode_it->begin("FREQ")[0]->to_string();
  std::set<double> freq_set;
  for(int i=0;i<bands.size();i++){ 
    for (Vex::Node::iterator chan_it = root_node["FREQ"][freq_node]->begin("chan_def");
         chan_it != root_node["FREQ"][freq_node]->end("chan_def"); ++chan_it) {
      double f = (*chan_it)[1]->to_double_amount("MHz")*1000000;
      if(bands[i] == f){
        subband new_band;
        new_band.frequency = (*chan_it)[1]->to_double_amount("MHz");
        new_band.bandwidth = chan_it[3]->to_double_amount("MHz");
        new_band.sideband = (chan_it[2]->to_string() == std::string("L"))? 0:1;
        int nbands = vex_subbands.size();
        vex_subbands.resize(nbands+1);
        if((nbands > 0) && (new_band.sideband == 0) && (vex_subbands[nbands-1].frequency == f)){
          // The upper sideband is allready in the list
          vex_subbands[nbands] = vex_subbands[nbands-1];
          vex_subbands[nbands-1] = new_band;
        }else{
          vex_subbands[nbands] = new_band;
        }
      }
    }
  }
}

void 
SFXCdata::src_coords(const string &src_name){
  // Store coordinates of src_name in variables src_ra and src_dec as strings
  int d, h, m;
  float s;
  string vra = vex.get_root_node()["SOURCE"][src_name]["ra"]->to_string();
  sscanf(vra.c_str(), "%dh%dm%fs", &h, &m, &s);
  char buf[14];
  sprintf(buf, "%02d:%02d:%07.4f", h, m, s);
  src_ra = string(buf);

  string vdec = vex.get_root_node()["SOURCE"][src_name]["dec"]->to_string();
  cout << "src_name = " << src_name << ", vra = " << vra << ", vdec = " << vdec << "\n";
  sscanf(vdec.c_str(), "%dd%d\'%fs\"", &d, &m, &s);
  // The format is (-dd:mm:ss.sss) 
  sprintf(buf,"%02d:%02d:%06.3f", d, m, s);
  //if (d > 0)
  //  sprintf(buf,"-%02d:%02d:%06.3f", d, m, s);
  //else
  //  sprintf(buf,"%02d:%02d:%06.3f", -d, m, s);
  src_dec = string(buf);
}

void 
SFXCdata::read_integration(int int_nr){
  cout << "Read integration " << int_nr << "\n";
  // Skip to desired integration
  long offset = (int_nr - (current_int + 1)) * sizeof_integration;
  fseek(infile, offset, SEEK_CUR);
  // Store array of dimensions (#subbands, #samples, #pol, #chan) as one dimensional array
  const int nchan = gheader.number_channels;
  const size_t ndata = nsamples * subbands.size() * npol * nchan;
  data.resize(ndata);
  // Read the data
  Output_header_timeslice htimeslice;
  size_t nbytes = fread(&htimeslice, 1, sizeof(htimeslice), infile);
  const int data_size = (nchan+1) * sizeof(float);
  cout << "number_baselines = " << htimeslice.number_baselines << ", htimeslice.integration_slice="
       <<htimeslice.integration_slice<<", int_nr =" <<int_nr <<"\n";
  while ((nbytes == sizeof(htimeslice)) && (int_nr == htimeslice.integration_slice)){
    Output_header_baseline hbaseline;
    Output_uvw_coordinates huvw;
    Output_header_bitstatistics hstatistics;
    float buffer[nchan+1];
    for(int i=0; i< htimeslice.number_uvw_coordinates; i++)
      nbytes = fread(&huvw, 1, sizeof(huvw), infile);
    for(int i=0; i< htimeslice.number_statistics; i++)
      nbytes = fread(&hstatistics, 1, sizeof(hstatistics), infile);
    int sample = 0;
    for(int i=0; i<htimeslice.number_baselines; i++){
      nbytes = fread(&hbaseline, 1, sizeof(hbaseline), infile);
      nbytes = fread(buffer, 1, data_size, infile);
      int f = subbands_index[hbaseline.sideband][hbaseline.frequency_nr];
      //cout << "subband_nr = " << (int)hbaseline.frequency_nr << ", sb=" << (int)hbaseline.sideband 
      //     << ", freq = " << f << ", pol = " << (int) hbaseline.polarisation1 << "\n";
      int pol=0;
      if (npol == 2)
        pol = hbaseline.polarisation1; 
      int nsubband = subbands.size();
      int idx = (f * nsamples + sample) * npol * nchan + pol * nchan;
      if(hbaseline.sideband == 0){
        // LSB
        for (int j = nchan; j > 0; j--)
          data[idx++] = buffer[j];
      }else{
        // USB
        for (int j = 0; j < nchan; j++)
          data[idx++] = buffer[j];
      }
      sample += 1;
    }
    nbytes = fread(&htimeslice, 1, sizeof(htimeslice), infile);
  }
  // skip back to start of time slice
  fseek(infile, -nbytes, SEEK_CUR);
  current_int = int_nr;
}

float *
SFXCdata::get_spectrum(int int_nr, int subband_nr, int sample, int pol_nr){
  if (int_nr != current_int)
    read_integration(int_nr);
  
  int nchan = gheader.number_channels;
  int pol = (npol == 1) ? 0 : pol_nr;
  int idx = subband_nr * nchan * npol * nsamples + (sample*npol + pol) * nchan;
//  cout << "int_nr = " << int_nr << ", subband_nr = " << subband_nr << ", sample = " << sample << ", pol_nr = " << pol_nr 
//      << ", idx = " << idx << ", size = " << data.size()<< "\n";
  return &data[idx];
}

void equatorial2galactic(double ra, double dec, double &glon, double &glat){
  double sind = sin(dec * M_PI / 180);
  double cosd = cos(dec * M_PI / 180);
  double cosa = cos(27.4 * M_PI / 180);
  double sina = sin(27.4 * M_PI / 180);
  double sinb = cosd * cosa * cos(ra-192.25*M_PI/180) + sind * sina;
  glat = asin(sinb);
  glon = atan2(sind-sinb*sina, cosd*sin(ra-192.26*M_PI/180)*cosa)*180/M_PI + 33;
}

string vex2datetime(int year, int yday, int sec_day){
  int months1[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int months2[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
  int *months = year%4 == 0 ? months2 : months1;
  int m, s;
  for(m=0, s=1;(m<12) && (s+months[m] <= yday); m++)
    s += months[m];
  int day = yday - s + 1;
  int hour = sec_day / (60*60);
  int minute = (sec_day - hour * 60 *60) / 60;
  int sec = sec_day % 60;
  char psr_time[20];
  sprintf(psr_time, "%d-%02d-%02dT%02d:%02d:%02d", year, m+1, day, hour, minute, sec);
  return string(psr_time);
}

void hdu_pad(FILE *outfile, uint8_t val){
  long pos = ftell(outfile);
  long mpos = pos%(FITS_HDU_QUANTUM*80);
  if (mpos == 0)
    return;
  
  vector<uint8_t> buf(FITS_HDU_QUANTUM*80 - mpos, val);
  fwrite(&buf[0], 1, FITS_HDU_QUANTUM*80 - mpos, outfile);
}

void hdr_int(FILE *outfile, const char *keyword, int val, const char *comment){
  fprintf(outfile, "%-8s= ", keyword);
  fprintf(outfile, "%20d", val);
  fprintf(outfile, " / %-47s", comment);
}

void hdr_float(FILE *outfile, const char *keyword, double val, int precision, const char *comment){
  fprintf(outfile, "%-8s= ", keyword);
  char fmt[8];
  sprintf(fmt, "%%20.%df", precision);
  fprintf(outfile, fmt, val);
  fprintf(outfile, " / %-47s", comment);
}
void hdr_bool(FILE *outfile, const char *keyword, bool val, const char *comment){
  fprintf(outfile, "%-8s= ", keyword);
  char b = val ? 'T': 'F';
  fprintf(outfile, "%20c", b);
  fprintf(outfile, " / %-47s", comment);
}

void hdr_string(FILE *outfile, const char *keyword, const string &str, const char *comment){
  fprintf(outfile, "%-8s= ", keyword);
  char fmt[16], buf[71];
  snprintf(buf, 71, "\'%s\'", str.c_str());
  int len = strlen(buf);
  sprintf(fmt, "%%-%ds", max(len, 20));
  fprintf(outfile, fmt, buf);
  sprintf(fmt, " / %%-%ds", min(67-len, 47));
  fprintf(outfile, fmt, comment);
}

void hdr_comment(FILE *outfile, const char *comment){
  fprintf(outfile, "COMMENT %-72s", comment);
}

void hdr_end(FILE *outfile){
  const char *end = "END";
  fprintf(outfile, "%-80s", end);
}

void write_subint_hdu(FILE *outfile, SFXCdata &sfxcdata, vector<int> &channels, int polarization){
  struct Output_header_global &gheader = sfxcdata.gheader;
  hdr_string(outfile, "XTENSION", "BINTABLE", "***** Subintegration data  *****");
  hdr_int(outfile, "BITPIX", 8, "N/A");
  hdr_int(outfile, "NAXIS", 2, " 2-dimensional binary table");
  int nsubband = channels.size();
  int nchan = gheader.number_channels*nsubband;
  // FIXME : Only a single polarization is written
  //int size_of_row = sizeof(double)*10 + sizeof(float)*(5+nchan*2*(1+sfxcdata.npol)) + sfxcdata.nsamples * nchan*sfxcdata.npol;
  int size_of_row = sizeof(double)*10 + sizeof(float)*(5+nchan*2*(1+1)) + (sfxcdata.nsamples) * nchan; 
  hdr_int(outfile, "NAXIS1", size_of_row, "width of table in bytes");
  hdr_int(outfile, "NAXIS2", sfxcdata.nsubint, "Number of rows in table (NSUBINT)");
  hdr_int(outfile, "PCOUNT", 0, "size of special data area");
  hdr_int(outfile, "GCOUNT", 1, "one data group (required keyword)");
  hdr_int(outfile, "TFIELDS", 20, "Number of fields per row");
 
  hdr_string(outfile, "INT_TYPE", "TIME", "Time axis (TIME, BINPHSPERI, BINLNGASC, etc)");
  hdr_string(outfile, "INT_UNIT", "SEC", "Unit of time axis (SEC, PHS (0-1), DEG)");
  hdr_string(outfile, "SCALE", "UNCAL", "Intensity units (FluxDen/RefFlux/Jansky)");
  hdr_string(outfile, "POL_TYPE", "INTEN", "Polarisation identifier (e.g., AABBCRCI, AA+BB)");
  // FIXME : Only a single polarization is written
  //hdr_int(outfile, "NPOL", sfxcdata[0].npol, "Nr of polarisations");
  hdr_int(outfile, "NPOL", 1, "Nr of polarisations");
  double tbin = gheader.integration_time / (1000000. * (sfxcdata.nsamples)); 
  hdr_float(outfile, "TBIN", tbin, 6, "[s] Time per bin or sample");
  hdr_int(outfile, "NBIN", 1, "Nr of bins (PSR/CAL mode; else 1)");
  hdr_int(outfile, "NBIN_PRD", 1, "Nr of bins/pulse period (for gated data)");
  hdr_float(outfile, "PHS_OFFS", 0, 1, "Phase offset of bin 0 for gated data");
  hdr_int(outfile, "NBITS", 8, "Nr of bits/datum (SEARCH mode data, else 1)");
  hdr_int(outfile, "ZERO_OFF", 0, "Zero offset for SEARCH-mode data");
  hdr_int(outfile, "SIGNINT", 0, "1 for signed ints in SEARCH-mode data, else 0");
  // Determine the number of subints before this scan
  string start_time = sfxcdata.vex.get_start_time_of_experiment();
  int ref_year, ref_day, ref_time;
  vex_time(start_time.c_str(), ref_year, ref_day, ref_time);
  double days_in_sec = (gheader.start_day - ref_day)*86400;
  double int_time =  gheader.integration_time / 1000000.;
  int nsuboffs = (int) round((days_in_sec + gheader.start_time  - ref_time) / int_time);

  // Determine the total bandwidth
  double bandwidth=0;
  for(int i=0;i<nsubband;i++)
    bandwidth += sfxcdata.subbands[channels[i]].bandwidth;
  hdr_int(outfile, "NSUBOFFS", nsuboffs, "Subint offset (Contiguous SEARCH-mode files)");
  hdr_int(outfile, "NCHAN", nchan, "Number of channels/sub-bands in this file");
  hdr_float(outfile, "CHAN_BW", bandwidth / nchan,3, "[MHz] Channel/sub-band width");
  hdr_float(outfile, "DM", 0, 2, "[cm-3 pc] DM for post-detection dedisperion");
  hdr_float(outfile, "RM", 0, 2, "[rad m-2] RM for post-detection deFaraday");
  hdr_int(outfile, "NCHNOFFS", 0, "Channel/sub-band offset for split files");
  hdr_int(outfile, "NSBLK", sfxcdata.nsamples, "Samples/row (SEARCH mode, else 1)"); 

  hdr_string(outfile, "EXTNAME", "SUBINT", "name of this binary table extension");

  hdr_string(outfile, "TTYPE1", "INDEXVAL", "Optionally used if INT_TYPE != TIME");
  hdr_string(outfile, "TFORM1", "1D", "Double");
  hdr_string(outfile, "TTYPE2", "TSUBINT", "Length of subintegration");
  hdr_string(outfile, "TFORM2", "1D", "Double");
  hdr_string(outfile, "TUNIT2", "s", "Units of field");
  hdr_string(outfile, "TTYPE3", "OFFS_SUB", "Offset from Start of subint centre");
  hdr_string(outfile, "TFORM3", "1D", "Double");
  hdr_string(outfile, "TUNIT3", "s", "Units of field");
  hdr_string(outfile, "TTYPE4", "LST_SUB", "LST at subint centre");
  hdr_string(outfile, "TFORM4", "1D", "Double");
  hdr_string(outfile, "TUNIT4", "s", "Units of field");
  hdr_string(outfile, "TTYPE5", "RA_SUB", "RA (J2000) at subint centre");
  hdr_string(outfile, "TFORM5", "1D", "Double");
  hdr_string(outfile, "TUNIT5", "deg", "Units of field");
  hdr_string(outfile, "TTYPE6", "DEC_SUB", "Dec (J2000) at subint centre");
  hdr_string(outfile, "TFORM6", "1D", "Double");
  hdr_string(outfile, "TUNIT6", "deg", "Units of field");
  hdr_string(outfile, "TTYPE7", "GLON_SUB", "[deg] Gal longitude at subint centre");
  hdr_string(outfile, "TFORM7", "1D", "Double");
  hdr_string(outfile, "TUNIT7", "deg", "Units of field");
  hdr_string(outfile, "TTYPE8", "GLAT_SUB", "[deg] Gal latitude at subint centre");
  hdr_string(outfile, "TFORM8", "1D", "Double");
  hdr_string(outfile, "TUNIT8", "deg", "Units of field");
  hdr_string(outfile, "TTYPE9", "FD_ANG", "[deg] Feed angle at subint centre");
  hdr_string(outfile, "TFORM9", "1E", "Float");
  hdr_string(outfile, "TUNIT9", "deg", "Units of field");
  hdr_string(outfile, "TTYPE10", "POS_ANG", "[deg] Position angle of feed at subint centre");
  hdr_string(outfile, "TFORM10", "1E", "Float");
  hdr_string(outfile, "TUNIT10", "deg", "Units of field");
  hdr_string(outfile, "TTYPE11", "PAR_ANG", "[deg] Parallactic angle at subint centre");
  hdr_string(outfile, "TFORM11", "1E", "Float");
  hdr_string(outfile, "TUNIT11", "deg", "Units of field");
  hdr_string(outfile, "TTYPE12", "TEL_AZ", "[deg] Telescope azimuth at subint centre");
  hdr_string(outfile, "TFORM12", "1E", "Float");
  hdr_string(outfile, "TUNIT12", "deg", "Units of field");
  hdr_string(outfile, "TTYPE13", "TEL_ZEN", "[deg] Telescope zenith angle at subint centre");
  hdr_string(outfile, "TFORM13", "1E", "Float");
  hdr_string(outfile, "TUNIT13", "deg", "Units of field");
  hdr_string(outfile, "TTYPE14", "AUX_DM", "additional DM (ionosphere, corona, etc.)");
  hdr_string(outfile, "TFORM14", "1D", "Double");
  hdr_string(outfile, "TUNIT14", "CM-3 PC", "units of field");
  hdr_string(outfile, "TTYPE15", "AUX_RM", "additional RM (ionosphere, corona, etc.)");
  hdr_string(outfile, "TFORM15", "1D", "Double");
  hdr_string(outfile, "TTYPE16", "DAT_FREQ", "[MHz] Centre frequency for each channel");
  char buf[24];
  sprintf(buf, "%dE", nchan);
  hdr_string(outfile, "TFORM16",  buf, "NCHAN floats");
  hdr_string(outfile, "TUNIT16", "MHz", "Units of field");
  hdr_string(outfile, "TTYPE17", "DAT_WTS", "Weights for each channel");
  hdr_string(outfile, "TFORM17", buf, "NCHAN floats");
  // FIXME : Only a single polarization
  //sprintf(buf, "%dE", nchan*sfxcdata.npol);
  sprintf(buf, "%dE", nchan);
  hdr_string(outfile, "TTYPE18", "DAT_OFFS", "Data offset for each channel");
  hdr_string(outfile, "TFORM18", buf, "NCHAN*NPOL floats");
  hdr_string(outfile, "TTYPE19", "DAT_SCL", "Data scale factor (outval=dataval*scl + offs)");
  hdr_string(outfile, "TFORM19", buf, "NCHAN*NPOL floats");
  hdr_string(outfile, "TTYPE20", "DATA", "Subint data table");
  // FIXME : Only a single polarization is written
  //sprintf(buf, "(%d, %d, %d)", nchan, sfxcdata.npol, sfxcdata.nsamples);
  sprintf(buf, "(%d, %d, %d)", nchan, 1, sfxcdata.nsamples); 
  hdr_string(outfile, "TDIM20", buf, "Dimensions (NBIN,NCHAN,NPOL/NCHAN,NPOL,NSBLK)");
  // FIXME : Only a single polarization is written
  //sprintf(buf, "%dB", nchan * sfxcdata[0].npol * sfxcdata[0].nsamples); 
  sprintf(buf, "%dB", nchan * (sfxcdata.nsamples)); 
  hdr_string(outfile, "TFORM20", buf, "I (Fold), X (1-bit) or B (2-8 bit) Search");
  hdr_string(outfile, "TUNIT20", "Jy", "Units of subint data");
  hdr_end(outfile);
  hdu_pad(outfile, ' '); // pad header with spaces
}

size_t write_float_be(float fval[], size_t nval, FILE *outfile){
  // Write array of floats to file in big endian format
  size_t nbytes = 0;
  for(int i=0;i<nval;i++){
    uint32_t val_be = htobe32(*((uint32_t *) &fval[i]));
    nbytes += fwrite(&val_be, 1, sizeof(uint32_t), outfile);
  }
  return nbytes;
}

size_t write_double_be(double fval[], size_t nval, FILE *outfile){
  // Write array of doubles to file in big endian format
  size_t nbytes = 0;
  for(int i=0;i<nval;i++){
    uint64_t val_be = htobe64(*((uint64_t *) &fval[i]));
    nbytes += fwrite(&val_be, 1, sizeof(uint64_t), outfile);
  }
  return nbytes;
}

void fits_subint_row_meta(double row_nr, int subint, FILE *outfile, SFXCdata &sfxcdata, vector<int> &channels, int ref_day, int ref_time, float weight, int fft_size){
  // Start a new row in the subint table, this function writes all columns except the last column 
  // containing the actual data.
  double int_time = sfxcdata.gheader.integration_time;
  // row_nr is optionally used if INT_TYPE != TIME
  size_t total = write_double_be(&row_nr, 1, outfile);
  // Length of subintegration
  double tsubint = int_time / 1000000;
  total += write_double_be(&tsubint, 1, outfile);
  // PRESTO seems to expect this 
  double offs_sub = int_time * (subint + 0.5) / 1000000;
  total += write_double_be(&offs_sub, 1, outfile);
  // LST at subint centre
  int ndays = floor((ref_time + ((subint + 0.5) * int_time) / 1000000) / 86400);
  double lst_sub = ref_time + ((subint + 0.5) * int_time) / 1000000 - ndays*86400; // FIXME : Not true sidereal time 
  total += write_double_be(&lst_sub, 1, outfile);
  // RA (J2000) at subint centre
  int d, m;
  double s;
  sscanf(sfxcdata.src_ra.c_str(),"%d:%d:%lf", &d, &m, &s);
  double ra_sub = 15*d + ((double)m + s / 60) / 4;
  total += write_double_be(&ra_sub, 1, outfile);
  // Dec (J2000) at subint centre
  sscanf(sfxcdata.src_dec.c_str(),"%d:%d:%lf", &d, &m, &s);
  double dec_sub = d + ((double)m + s / 60) /60;
  total += write_double_be(&dec_sub, 1, outfile);
  // [deg] Gal longitude at subint centre
  double glon, glat;
  equatorial2galactic(ra_sub, dec_sub, glon, glat);
  total += write_double_be(&glon, 1, outfile);
  // [deg] Gal latitude at subint centre
  total += write_double_be(&glat, 1, outfile);
  // [deg] Feed angle at subint centre
  float fd_ang = 0;
  total += write_float_be(&fd_ang, 1, outfile);
  // [deg] Position angle of feed at subint centre
  float pos_ang = 0;
  total += write_float_be(&pos_ang, 1, outfile);
  // [deg] Parallactic angle at subint centre
  float par_ang = 0;
  total += write_float_be(&par_ang, 1, outfile);
  // [deg] Telescope azimuth at subint centre
  float tel_az = 0; 
  total += write_float_be(&tel_az, 1, outfile);
  // [deg] Telescope zenith angle at subint centre
  float tel_zen = 0; 
  total += write_float_be(&tel_zen, 1, outfile);
  // additional DM (ionosphere, corona, etc.)
  double aux_dm = 0;
  total += write_double_be(&aux_dm, 1, outfile);
  // additional RM (ionosphere, corona, etc.)
  double aux_rm = 0;
  total += write_double_be(&aux_rm, 1, outfile);
  int nsubbands = channels.size();
  int nchan = sfxcdata.gheader.number_channels;
  for(int ch_idx=0;ch_idx<nsubbands;ch_idx++){
    int ch = channels[ch_idx];
    double f0 = sfxcdata.subbands[ch].frequency; 
    double bandwidth = sfxcdata.subbands[ch].bandwidth;
    int sb = sfxcdata.subbands[ch].sideband*2-1; // LSB: -1, USB: 1
    // If the fft_size if the correlation was larger than nchan, then all
    // frequencies are shifted by an amount delta
    int ratio = fft_size/nchan;
    float delta= (ratio-1) * bandwidth / (2*fft_size);
    if(sb==-1){
      for(int i=nchan;i>0;i--){
        float f = f0 + delta - i*bandwidth/nchan;
        total += write_float_be(&f, 1, outfile);
      }
    }else{
      for(int i=0;i<nchan;i++){
        float f = f0 + delta + i*bandwidth/nchan;
        total += write_float_be(&f, 1, outfile);
      }
    }
  }
  // Weights for each channel
  // All channels get the same weight, for now
  for(int i=0;i<nchan*nsubbands;i++){
    total += write_float_be(&weight, 1, outfile);
  }
  // Data offset for each channel
  float off = 0; 
  // FIXME : Only a single polarization
  //for(int i=0;i<nchan*sfxcdata.npol;i++)
  for(int i=0;i<nchan*nsubbands;i++){
    total += write_float_be(&off, 1, outfile);
  }
  // Data scale factor (outval=dataval*scl + offs)
  float scl = 1; 
  //FIXME : Only a single polarization
  // for(int i=0;i<nchan*sfxcdata.npol;i++)
  for(int i=0;i<nchan*nsubbands;i++){
    total += write_float_be(&scl, 1, outfile);
  }
}

void get_quantization(SFXCdata &sfxcdata, vector<int> &channels, int polarization, float lower_bound[], float step[]){
  // Get the quantiation levels. Each sub-integration is a sum of Rayleigh distributed samples of the
  // signal power. Because of the central limit theorem this sum should be approximately gaussian.
  const int nint = sfxcdata.nsubint;
  const int nsamples = sfxcdata.nsamples;
  const int nchan = sfxcdata.gheader.number_channels;
  const int nfreq = channels.size();
 
  const int pol = (polarization == POL_I) ? 0 : polarization;
  
  const int nint_in_8sec = (int) round(8000000./sfxcdata.gheader.integration_time);
  const int bint = max(nint/2 - nint_in_8sec, 0);
  const int eint = min(bint+2*nint_in_8sec, nint-1);
  
  const int N = (eint-bint+1) * nsamples;
  vector<vector<float> > buf(nchan*nfreq);
  for(int i=0;i<nchan*nfreq;i++)
    buf[i].resize(N);

  // Copy the data range to an auxilary array
  for(int int_nr = bint; int_nr <= eint; int_nr++){
    //std::cout << "int_nr = " << int_nr << " / " << eint << "\n";
    for(int freq_nr = 0; freq_nr < nfreq; freq_nr++){
      int freq = channels[freq_nr];
      for(int sample = 0; sample < nsamples; sample++){
        int j = (int_nr-bint)*nsamples+sample;
        float *data_row = sfxcdata.get_spectrum(int_nr, freq, sample, pol);
        for(int i=0; i<nchan; i++)
          buf[freq_nr*nchan+i][j] = data_row[i];
        if (polarization == POL_I){
          float *data_row = sfxcdata.get_spectrum(int_nr, freq, sample, 1-pol);
          for(int i=0; i<nchan; i++)
            buf[freq_nr*nchan+i][j] += data_row[i];
        }
      }
    }
  }
  for(int i=0;i<nchan*nfreq;i++){
    //Sort all data
    sort(buf[i].begin(), buf[i].end());
    // Filter the zeros
    int start=0;
    while((buf[i][start] < -1) && (start < buf[i].size()-1))
      start += 1;
    int end = start;
    while((buf[i][end] < 1) && (end < buf[i].size()-1))
      end += 1;
    // Copy the non zeros
    for(int j=0;j<N-end;j++)
      buf[i][start+j] = buf[i][end+j];
    int M = N-(end-start);
    double median = buf[i][M/2];
    double average = 0, average_squares=0;
    for(int j=0;j<M;j++){
      average += buf[i][j];
      average_squares += buf[i][j]*buf[i][j];
    }
    average /= M;
    average_squares /= M;
    double sigma = sqrt(average_squares - average*average);
    // From Jenet and Anderson, 1998
    step[i] = sigma * 0.02957;
    lower_bound[i] = median - step[i] *127;
    //std::cout << i << " : step = " << step[i] << ", median = " << median << ", average = " << average << ", av_sq = " << average_squares << "\n";
  }
}

void get_quantization2(SFXCdata &sfxcdata, vector<int> &channels, float lower_bound[], float step[]){
  // Get the quantiation levels. Each sub-integration is a sum of Rayleigh distributed samples of the
  // signal power. Because of the central limit theorem this sum should be approximately gaussian.
  const int nint = sfxcdata.nsubint;
  const int nsamples = sfxcdata.nsamples;
  const int nchan = sfxcdata.gheader.number_channels;
  const int nfreq = channels.size();
  
  const int pol = 0;  // Use a single polarization

  const int nint_in_8sec = (int) round(8000000./sfxcdata.gheader.integration_time);
  const int bint = max(nint/2 - nint_in_8sec, 0);
  const int eint = min(bint+2*nint_in_8sec, nint-1);
  
  const int N = (eint-bint+1) * nsamples;
  vector<vector<float> > buf(nchan*nfreq);
  for(int i=0;i<nchan*nfreq;i++)
    buf[i].resize(N);

  // Copy the data range to an auxilary array
  std::cout << "nsamples = " << nsamples << ", nchan = " << nchan << ", nfreq = " << nfreq<< "\n";
  for(int int_nr = bint; int_nr <= eint; int_nr++){
    std::cout << "int_nr = " << int_nr << " / " << eint << "\n";
    for(int freq_nr = 0; freq_nr < nfreq; freq_nr++){
      int freq = channels[freq_nr];
      for(int sample = 0; sample < nsamples; sample++){
        int j = (int_nr-bint)*nsamples+sample;
        float *data_row = sfxcdata.get_spectrum(int_nr, freq, sample, pol);
        for(int i=0; i<nchan; i++)
          buf[freq_nr*nchan+i][j] = data_row[i];
      }
    }
  }
  for(int i=0;i<nchan*nfreq;i++){
    //Sort all data
    sort(buf[i].begin(), buf[i].end());
    // Alternative method : just pick the numbers from the buffer select up to 10^-4
    lower_bound[i] = buf[i][N / 1000];
    // Fixme : this was 10^-4
    step[i] = (buf[i][N-N/1000-1] - lower_bound[i]) / 255;
  }
}

void vex_time(const char *time_str, int &ref_year, int &ref_day, int &ref_time){
  //2008y234d12h12m34s
  int minute, hour, second;
  sscanf(time_str, "%dy%dd%dh%dm%ds", &ref_year, &ref_day, &hour, &minute, &second);
  ref_time = second + 60*(hour*60+minute);
}

void write_subints(FILE *outfile, SFXCdata &sfxcdata, vector<int> &channels, int polarization, int fft_size){
  // Requantize subintgrations to 8 bit and write the results to disk.
  string start_time_str = sfxcdata.vex.get_start_time_of_experiment();
  int ref_year, ref_day, ref_time;
  vex_time(start_time_str.c_str(), ref_year, ref_day, ref_time);
  double int_time = sfxcdata.gheader.integration_time / 1000000.;
  int start_day = sfxcdata.gheader.start_day;
  int start_time = sfxcdata.gheader.start_time;
  int int_nr = round(((start_day-ref_day)*86400 + (start_time - ref_time))/int_time);

  // FIXME : Only single pol is supported
  const int nsubint = sfxcdata.nsubint, nsamples = sfxcdata.nsamples, npol = 1; // data->npol;
  const int nchan = sfxcdata.gheader.number_channels;
  const int nfreq = channels.size();
  float lower_bound[nchan*nfreq], dlevel[nchan*nfreq];
  get_quantization(sfxcdata, channels, polarization, lower_bound, dlevel);
  flag = true;
  //cout << "dlevel = " << dlevel[nchan/2] << ", lower bound = " << lower_bound[nchan/2] 
  //     << ", nchan = " << nchan << ", nfreq= " << nfreq<< "\n";
  // Now write the data (requantized to 8bit), for now just the first channel
  uint8_t buf[nchan];//buf[nsamples];
  for(int subint_nr = 0; subint_nr < nsubint; subint_nr++){
    fits_subint_row_meta(subint_nr, int_nr, outfile, sfxcdata, channels, ref_day, ref_time, 1., fft_size); // start new row
    size_t nbytes = 0;
    for(int sample = 0; sample < nsamples; sample++){
      int pol = (polarization == POL_I) ? 0 : polarization; // FIXME : Only single polarization is supported
      for(int sb_idx=0;sb_idx<nfreq;sb_idx++){
        int freq = channels[sb_idx];
        // FIXME CLean up the implementation of polarization summing
        float *data_row = sfxcdata.get_spectrum(subint_nr, freq, sample, pol);
        float *data_row2 = (polarization != POL_I) ? NULL : sfxcdata.get_spectrum(subint_nr, freq, sample, 1-pol);
        for(int ch = 0; ch < nchan; ch++){
          int idx = sb_idx*nchan + ch;
          int val;
          if (data_row2 == NULL)
            val = round((data_row[ch]-lower_bound[idx]) / dlevel[idx]);
          else
            val = round((data_row[ch]+data_row2[ch]-lower_bound[idx]) / dlevel[idx]);
          buf[ch] = max(min(val, 255), 0);
          if ((subint_nr == -10) && (ch==48))
            cout << "buf["<<idx<<"]="<<(int)buf[ch]<< ", orig = " << val  << ", dlevel = " << dlevel[idx] << ", lower = " << lower_bound[idx] << ", data = " << data_row[ch] 
                 << "\n";
        }
        //nbytes += fwrite(buf, 1, nsamples, outfile);
        nbytes += fwrite(buf, 1, nchan, outfile);
      }
    }
    cout << "Wrote " << nbytes << " bytes of data in integration " << int_nr<< "\n";
    int_nr++;
  }
  // Pad the table, the table length has to be a multiple of the HDU quantum
  hdu_pad(outfile, 0);
}

void write_primary_hdu(FILE *outfile, SFXCdata &sfxcdata, vector<int> &channels, int polarization){
  const Output_header_global &gheader = sfxcdata.gheader;
  time_t rawtime;
  struct tm * timeinfo;
  char temp[81];

  hdr_bool(outfile, "SIMPLE", true, "file does conform to FITS standard"); 
  hdr_int(outfile, "BITPIX", 8, "number of bits per data pixel");
  hdr_int(outfile, "NAXIS", 0, "number of data axes");
  hdr_bool(outfile, "EXTEND", true, "FITS dataset may contain extensions"); 
  hdr_comment(outfile, "FITS (Flexible Image Transport System) format defined in Astronomy and");
  hdr_comment(outfile, "Astrophysics Supplement Series v44/p363, v44/p371, v73/p359, v73/p365."); 
  hdr_comment(outfile, "Contact the NASA Science Office of Standards and Technology for the"); 
  hdr_comment(outfile, "FITS Definition document #100 and other FITS information."); 
  hdr_comment(outfile, ""); 
  hdr_string(outfile, "HDRVER", "5.0", "Header version");
  hdr_string(outfile, "FITSTYPE", "PSRFITS", "FITS definition for pulsar data files");

  // Creation time
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  sprintf(temp, "%d-%02d-%02dT%02d:%02d:%02d", 1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday, 
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  hdr_string(outfile, "DATE", temp, "File creation date (YYYY-MM-DDThh:mm:ss UTC)");
  hdr_string(outfile, "OBSERVER", sfxcdata.pi_name, "Observer name(s)");
  hdr_string(outfile, "PROJID", sfxcdata.exper_name, "Project name");
  hdr_string(outfile, "TELESCOP", "Geocenter", "Telescope name");

  // Virtual telescope is at geocenter
  hdr_float(outfile, "ANT_X", 0, 1, "[m] Antenna ITRF X-coordinate (D)");
  hdr_float(outfile, "ANT_Y", 0, 1, "[m] Antenna ITRF Y-coordinate (D)");
  hdr_float(outfile, "ANT_Z", 0, 1, "[m] Antenna ITRF Z-coordinate (D)");
  hdr_string(outfile, "FRONTEND", "PHASED", "Receiver ID");
  hdr_int(outfile, "IBEAM", 1, "Beam ID for multibeam systems");

  // FIXME : Only a polarization can be used 
  //hdr_int(outfile, "NRCVR", sfxcdata.npol, "Number of receiver polarisation channels"); 
  hdr_int(outfile, "NRCVR", 1, "Number of receiver polarisation channels"); 
  
  hdr_string(outfile, "FD_POLN", "CIRC", "LIN or CIRC");
  hdr_int(outfile, "FD_HAND", 1, "+/- 1. +1 is LIN:A=X,B=Y, CIRC:A=L,B=R (I)"); 
  hdr_int(outfile, "FD_SANG", 0, "[deg] FA of E vect for equal sig in A&B (E)"); 
  hdr_int(outfile, "FD_XYPH", 0, "[deg] Phase of A^* B for injected cal (E)"); 
  hdr_string(outfile, "BACKEND", "SFXC", "Backend ID");
  hdr_string(outfile, "BECONFIG", "NONE", "Backend configuration file name");
  hdr_int(outfile, "BE_PHASE", 0, "0/+1/-1 BE cross-phase:0 unknown,+/-1 std/rev");
  hdr_int(outfile, "BE_DCC", 0, "0/1 BE downconversion conjugation corrected"); 
  hdr_int(outfile, "BE_DELAY", 0, "[s] Backend propn delay from digitiser input"); 
  hdr_int(outfile, "TCYCLE", 0, "[s] On-line cycle time (D)"); 
  hdr_string(outfile, "OBS_MODE", "SEARCH", "(PSR, CAL, SEARCH)");
  // Get start time of experiment (start of first scan in vex file)
  string start_time = sfxcdata.vex.get_start_time_of_experiment();
  int ref_year, ref_day, ref_time;
  vex_time(start_time.c_str(), ref_year, ref_day, ref_time);
  string t = vex2datetime(ref_year, ref_day, ref_time);
  hdr_string(outfile, "DATE-OBS", t, "Date of observation (YYYY-MM-DDThh:mm:ss UTC");

  // Determine the centre frequency and the total bandwith. Because subbands don't have to be
  // continuous we compute this imformation from upper and lower frequency.
  int nsubband = channels.size();
  SFXCdata::subband &lowersb = sfxcdata.subbands[channels[0]];
  SFXCdata::subband &uppersb = sfxcdata.subbands[channels[nsubband-1]];
  double f0 = lowersb.frequency + lowersb.bandwidth*(2*lowersb.sideband-1);
  double f1 = uppersb.frequency + uppersb.bandwidth*(2*uppersb.sideband-1);
  double obsfreq = (f0+f1)/2;
  double bandwidth = f1-f0;
  hdr_float(outfile, "OBSFREQ", obsfreq, 2, "[MHz] Centre frequency for observation");
  hdr_float(outfile, "OBSBW", bandwidth, 1, "[MHz] Bandwidth for observation"); 
  hdr_int(outfile, "OBSNCHAN", gheader.number_channels*nsubband, "Number of frequency channels (original)"); 
  hdr_int(outfile, "CHAN_DM", 0, "[cm-3 pc] DM used for on-line dedispersion"); 
  hdr_string(outfile, "PNT_ID", "PNT_ID", "Name or ID for pointing ctr (multibeam feeds)");

  hdr_string(outfile, "SRC_NAME", sfxcdata.src_name, "Source or scan ID");
  hdr_string(outfile, "COORD_MD", "J2000", "Coordinate mode (J2000, GALACTIC, ECLIPTIC)");
  hdr_float(outfile, "EQUINOX", 2000, 1, "Equinox of coords (e.g. 2000.0)");
  hdr_string(outfile, "RA", sfxcdata.src_ra, "Right ascension (hh:mm:ss.ssss)");
  hdr_string(outfile, "DEC", sfxcdata.src_dec, "Declination (-dd:mm:ss.sss)");
  // TODO : Bogus values for now
  hdr_float(outfile, "BMAJ", 0.5, 1, "[deg] Beam major axis length"); 
  hdr_float(outfile, "BMIN", 0.5, 1, "[deg] Beam minor axis length"); 
  hdr_int(outfile, "BPA", 0, "[deg] Beam position angle"); 
  hdr_string(outfile, "STT_CRD1", sfxcdata.src_ra, "Start coord 1 (hh:mm:ss.sss or ddd.ddd)");
  hdr_string(outfile, "STT_CRD2", sfxcdata.src_dec, "Start coord 2 (-dd:mm:ss.sss or -dd.ddd)");
  // Leave empty
  hdr_string(outfile, "TRK_MODE", "TRACK   ", "Track mode (TRACK, SCANGC, SCANLAT)");
  hdr_string(outfile, "STP_CRD1", sfxcdata.src_ra, "Stop coord 1 (hh:mm:ss.sss or ddd.ddd)");
  hdr_string(outfile, "STP_CRD2", sfxcdata.src_dec, "Stop coord 2 (-dd:mm:ss.sss or -dd.ddd)");
  double scanlen = sfxcdata.nsubint * gheader.integration_time / 1000000.;
  hdr_float(outfile, "SCANLEN", scanlen, 1, "[s] Requested scan length (E)"); 
  hdr_string(outfile, "FD_MODE", "FA", "Feed track mode - FA, CPA, SPA, TPA");
  hdr_float(outfile, "FA_REQ", 0, 1, "[deg] Feed/Posn angle requested (E)"); 
  hdr_string(outfile, "CAL_MODE", "OFF", "Cal mode (OFF, SYNC, EXT1, EXT2)");
  hdr_int(outfile, "CAL_FREQ", 0, "[Hz] Cal modulation frequency (E)"); 
  hdr_int(outfile, "CAL_DCYC", 0, "Cal duty cycle (E"); 
  hdr_int(outfile, "CAL_PHS", 0, "Cal phase (wrt start time) (E)"); 
  hdr_int(outfile, "CAL_NPHS", 0, "Number of states in cal pulse (I)");
  // start date-time equal to that of the first scan in the vex file
  int stt_imjd = mjd(1, 1, ref_year) + ref_day - 1;
  hdr_int(outfile, "STT_IMJD", stt_imjd, "Start MJD (UTC days) (J - long integer)"); 
  hdr_float(outfile, "STT_SMJD", ref_time, 1, "[s] Start time (sec past UTC 00h) (J)"); 
  hdr_float(outfile, "STT_OFFS", 0, 1, "[s] Start time offset (D)");
  // LST = Local Sidereal Time ???
  hdr_float(outfile, "STT_LST", ref_time, 1, "[s] Start LST (D)");
  hdr_end(outfile); 
  hdu_pad(outfile, ' '); // pad with spaces
}

void usage(char *name){
  int i = strlen(name);
  while ((i > 0) && (name[i] != '/'))
    i--;
  cout << "Usage : " << &name[i] << "[OPTIONS] <vex file> <input file> <output file> \n\n"
       << "Options : \n"
       << "  -c CHANNELS       List of channels to include [Default All channels]\n" 
       << "  -f FFT_SIZE       FFT size which was used during correlation\n" 
       << "  -p POLARIZATIONS  Select polarizations : R, L, I  [Default I (sums R&L)]\n";
}

int main(int argc, char *argv[]){
  FILE *outfile;
  char *channels_str = NULL;
  char *pol_str = NULL;
  int fft_size = 0;
  uit = fopen("debug.txt", "w");
  // Parse command line arguments
  opterr = 0;
  int c;
  while ((c = getopt (argc, argv, "c:p:f:h")) != -1){
    switch (c){
      case 'p':
        pol_str = optarg;
        break;
      case 'c':
        channels_str = optarg;
        break;
      case 'f':
        fft_size = atoi(optarg);
        if(fft_size == 0){
          cerr << "Invalid parameter to option -f\n";
          exit(1);
        }
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
        break;
      case '?':
        if (optopt == 'c')
          cerr << "Option -c requires a list of channels.\n";
        else if (optopt == 'c')
          cerr << "Option -f requires parameter.\n";
        else if (optopt == 'p')
          cerr << "Option -p takes R, L, R&L, or I as argument.\n";
        else 
          cerr << "Unknown option -" << (char)optopt << "\n";
        exit(1);
    }
  }
  int narg = argc - optind;
  cout << "argc = " << argc << ", optind = " << optind << "\n";
  if((narg != 3) || (opterr != 0)){
    usage(argv[0]);
    exit(0);
  }
  // Open output file
  outfile = fopen(argv[argc-1], "w");
  if (!outfile){
    cerr << "Could not open " << argv[argc-1] << " for writing.\n";
    exit(1);
  }
  // Open SFXC correlator files, there can be more than one
  SFXCdata sfxcdata(argv[argc-2], argv[argc-3]);
  // Create a list of channels to be included in the PSRFITS file
  // NB: We assume all correlator files have the same amount of channels
  int nchannels = sfxcdata.subbands.size();
  std::vector<int> channels;
  if (channels_str == NULL){
    channels.resize(nchannels);
    for(int i = 0; i < nchannels; i++)
      channels[i] = i;    
  }else{
    int n = strlen(channels_str);
    set<int> channel_set;
    int first=0, last=0, *current=&first;
    for(int i=0;i<=n;i++){
      switch(channels_str[i]){
        case ',':
        case '\0':
          if(last<first)
            last=first;
            for(int j=first;j<=last;j++)
              channel_set.insert(j);
            first=0;
            last=0;
            current = &first;
          break;
        case ':':
        case '-':
          current = &last;
          break;
        default:
          if (!isdigit(channels_str[i])){
            cout << "Error in format of channel list.\n";
            exit(1);
          }
          *current = *current*10 + channels_str[i]-'0';
      }
    }
    for (set<int>::iterator it = channel_set.begin(); it !=channel_set.end(); it++){
      int ch = *it;
      if(ch >= sfxcdata.subbands.size()){
        cerr << "Error, invalid channel number\n";
        exit(1);
      }
      channels.push_back(ch);
    }
  }
  // Setting default fft_size if it wasn't passed as an parameter
  if (fft_size == 0)
    fft_size = sfxcdata.gheader.number_channels;
  if (fft_size < sfxcdata.gheader.number_channels){
    cerr << "Invalid fft_size, it cannot be smaller than the number of "
         << "channels in the data file. "
         << "NB: number_channels = " <<  sfxcdata.gheader.number_channels
         << "\n";
    exit(1);
  }
  // Get polarizations
  int pol;
  if ((pol_str == NULL) || (strcmp(pol_str, "I") == 0))
    pol = POL_I;
  else if (strcmp(pol_str, "R") == 0)
    pol = POL_R;
  else if (strcmp(pol_str, "L") == 0)
    pol = POL_L;
  else if (strcmp(pol_str, "R&L") == 0){
    cerr << "Sorry, we don't support more than one polarizaion yet\n";
    exit(1);
  }else{
    cerr << "Invalid polarization option : " << pol_str << "\n";
    exit(1);
  }

  // write the fits file
  std::cout << "primary hdi, pol="<<pol<< "\n";
  write_primary_hdu(outfile, sfxcdata, channels, pol);
  std::cout << "subin hdu\n";
  write_subint_hdu(outfile, sfxcdata, channels, pol);
  std::cout << "write_subints\n";
  write_subints(outfile, sfxcdata, channels, pol, fft_size);
  return 0;
}
