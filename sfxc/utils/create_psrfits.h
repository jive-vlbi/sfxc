#ifndef CREATE_PSRFITS_H
#define CREATE_PSRFITS_H
#include <vector>
#include <string>
#include <vex/Vex++.h>
#include "output_header.h"

#define FITS_ID_LENGTH 8
#define FITS_VALUE_POS 11
#define FITS_COMMENT_POS 32
#define FITS_HDU_QUANTUM 36

class SFXCdata{
public:
  struct subband{
    double frequency;
    double bandwidth;
    int sideband;
  };
public:
  SFXCdata(const char *infile, const char *vexname);
  float *get_spectrum(int int_nr, int subband_nr, int sample, int pol_nr);
private:
  void read_integration(int int_nr);
  void src_coords(const std::string &src_name);
  void get_subbands(std::vector<subband> &subbands);
  void get_parameters();
public:
  Vex vex;
  struct Output_header_global gheader;
  std::string pi_name, exper_name, src_name;
  std::string src_ra, src_dec; // Right ascention and declination of source
  int npol;  // Number of polarizations in file
  int nsamples; // Number of samples within a subint
  int nsubint;  // Number of integrations
  std::vector<int> subbands_index[2];
  std::vector<subband> subbands;
private:
  FILE *infile;
  std::vector<float> data; // data array
  int current_int;
  size_t sizeof_integration;
};

void equatorial2galactic(double ra, double dec, double &glon, double &glat);
std::string vex2datetime(int year, int yday, int sec_day);
void hdu_pad(FILE *outfile, uint8_t val);
void hdr_int(FILE *outfile, const char *keyword, int val, const char *comment);
void hdr_float(FILE *outfile, const char *keyword, double val, int precision, const char *comment);
void hdr_bool(FILE *outfile, const char *keyword, bool val, const char *comment);
void hdr_string(FILE *outfile, const char *keyword, const std::string &str, const char *comment);
void hdr_comment(FILE *outfile, const char *comment);
void hdr_end(FILE *outfile);
size_t write_float_be(float fval[], size_t nval, FILE *outfile);
size_t write_double_be(double fval[], size_t nval, FILE *outfile);
void fits_subint_row_meta(double row_nr, int subint, FILE *outfile, SFXCdata &sfxcdata, std::vector<int> &channels, int ref_day, int ref_time, float weight);
void vex_time(const char *start_time, int &ref_year, int &ref_day, int &ref_time);
void get_quantization(SFXCdata &sfxcdata, std::vector<int> &channels, float lower_bound[], float step[]);
void write_subints(FILE *outfile,SFXCdata &sfxcdata, std::vector<int> &channels);
void write_subint_hdu(FILE *outfile, SFXCdata &sfxcdata, std::vector<int> &channels);
void write_primary_hdu(FILE *outfile, SFXCdata &sfxcdata, std::vector<int> &channels);
void usage(char *name);
#endif
