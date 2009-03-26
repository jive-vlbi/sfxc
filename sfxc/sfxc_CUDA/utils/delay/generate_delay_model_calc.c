/*
  purpose    : delmo generates delay tables in sfxc type format according to 
  the parameters set in a delmo_control_file, which was generated 
  with the utility vex2ccf

  last change: 29-05-2007
  authors    : RHJ Oerlemans, M Kettenis

  dependencies: files ocean.dat tilt.dat and DE405_le.jpl should be in $HOME/bin.
*/

#include <generate_delay_model.h>

#include <sys/types.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Calc-10 constants
#include "calc10/include/constants.h"


// Output file
FILE *output_file;

// function to compute the delay correction
void calc();

int scan_nr = 0;    // Number of the scan being processed
int interval = 0;   // Interval number in the current scan

// Calculated delay.
double delay[2] = { NAN }; //sec
double uvw[3];



/** Fortran interface **/

void
ask(const char *name, short *ntoc, short *n1, short *n2, short *n3,
    short *nver, char *text, short *ktype, short *err)
{
  if (strncmp(name, "ROTEPOCH", 8) == 0) {
    *err = 1;
    return;
  }

  printf("%s: %.8s\n", __func__, name);
  *err = 1;
}

void
add4(void)
{
  // UNUSED
}

void
adda(void)
{
  // UNUSED
}

void
addi(void)
{
  // UNUSED
}

void
addr(void)
{
  // UNUSED
}

void
dela(void)
{
  // UNUSED
}

void
delr(void)
{
  // UNUSED
}

void
get4(const char *name, double *value, short *n1, short *n2, short *n3,
     short *ndo, short *err)
{
  int i;

  if (strncmp(name, "PRE DATA", 8) == 0) {
    *value = prec_const;
    *err = 0;
    return;
  }

  if (strncmp(name, "AXISOFFS", 8) == 0) {
    assert(*n1 == 2);
    value[0] = 0.0;
    value[1] = station_data.axis_offset;
    *err = 0;
    return;
  }

  if (strncmp(name, "SITEZENS", 8) == 0) {
    assert(*n1 == 2);
    value[0] = 0.0;
    value[1] = 0.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "SITERECV", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "SITERECS", 8) == 0) {
    assert(*n1 == 3);
    assert(*n2 == 2);
    value[0] = 0.0;
    value[1] = 0.0;
    value[2] = 0.0;
    value[3] = station_data.site_position[0];
    value[4] = station_data.site_position[1];
    value[5] = station_data.site_position[2];
    *err = 0;
    return;
  }

  if (strncmp(name, "SITOCAMP", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "SITOCPHS", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "SITHOCAM", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "SITHOCPH", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "STAR2000", 8) == 0) {
    assert(*n1 == 2);
    assert(*n2 == 1);
    value[0] = scan_data[scan_nr].ra;
    value[1] = scan_data[scan_nr].dec;
    *err = 0;
    return;
  }

  if (strncmp(name, "FUT1 INF", 8) == 0) {
    assert(*n1 == 4);
    value[0] = station_data.eop_ref_epoch;
    value[1] = 1.0;
    value[2] = station_data.num_eop_points;
    value[3] = 1.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "FUT1 PTS", 8) == 0) {
    assert(*n1 == station_data.num_eop_points);
    for (i = 0; i < station_data.num_eop_points; i++)
      value[i] = station_data.tai_utc - station_data.ut1_utc[i];
    *err = 0;
    return;
  }

  if (strncmp(name, "FWOB INF", 8) == 0) {
    assert(*n1 == 3);
    value[0] = station_data.eop_ref_epoch;
    value[1] = 1.0;
    value[2] = station_data.num_eop_points;
    *err = 0;
    return;
  }

  if (strncmp(name, "FWOBX&YT", 8) == 0) {
    assert(*n1 == 2);
    assert(*n2 == station_data.num_eop_points);
    for (i = 0; i < station_data.num_eop_points; i++) {
      value[i * 2] = station_data.x_wobble[i] * 1000.;
      value[i * 2 + 1] = station_data.y_wobble[i] * 1000.;
    }
    *err = 0;
    return;
  }

  if (strncmp(name, "TAI- UTC", 8) == 0) {
    assert(*n1 == 3);
    value[0] = station_data.eop_ref_epoch;
    value[1] = station_data.tai_utc;
    value[2] = 0.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "A1 - TAI", 8) == 0) {
    assert(*n1 == 3);
    value[0] = station_data.eop_ref_epoch;
    value[1] = 0.0;
    value[2] = 0.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "SEC TAG", 7) == 0) {
    value[0] = scan_data[scan_nr].sec;
    *err = 0;
    return;
  }

  if (strncmp(name, "REF FREQ", 8) == 0) {
    value[0] = NAN;
    /*         value[0] = station_data.freq; */
    *err = 0;
    return;
  }

  if (strncmp(name, "ATM PRES", 8) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "TEMP C", 6) == 0) {
    *err = 1;
    return;
  }

  if (strncmp(name, "REL.HUM.", 8) == 0) {
    *err = 1;
    return;
  }

  printf("%s: %.8s(%d, %d, %d)\n", __func__, name, *n1, *n2, *n3);
  *err = 1;
}

void
geta(const char *name, char value[][8], short *n1, short *n2, short *n3,
     short *ndo, short *err)
{
  if (strncmp(name, "SITNAMES", 8) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 2);
    strncpy(value[0], "C_OF_E", sizeof(value[0]));
    strncpy(value[1], station_data.site_name, sizeof(value[1]));
    *err = 0;
    return;
  }

  if (strncmp(name, "STRNAMES", 8) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 1);
    strncpy(value[0], scan_data[scan_nr].source_name, sizeof(value[0]));
    *err = 0;
    return;
  }

  if (strncmp(name, "EOPSCALE", 8) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 1);
    strncpy(value[0], "UTC     ", 8);
    *err = 0;
    return;
  }

  if (strncmp(name, "BASELINE", 8) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 2);
    strncpy(value[0], "C_OF_E", sizeof(value[0]));
    strncpy(value[1], station_data.site_name, sizeof(value[1]));
    *err = 0;
    return;
  }

  if (strncmp(name, "STAR ID", 7) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 1);
    strncpy(value[0], scan_data[scan_nr].source_name, sizeof(value[0]));
    *err = 0;
    return;
  }

  printf("%s: %.8s(%d, %d, %d)\n", __func__, name, *n1, *n2, *n3);
  *err = 1;
}

void
geti(const char *name, short *value, short *n1, short *n2, short *n3,
     short *ndo, short *err)
{
  if (strncmp(name, "# SITES", 7) == 0) {
    *value = 2;
    *err = 0;
    return;
  }

  if (strncmp(name, "AXISTYPS", 8) == 0) {
    assert(*n1 == 2);
    value[0] = 0;
    value[1] = station_data.axis_type;
    *err = 0;
    return;
  }

  if (strncmp(name, "INTRVAL4", 8) == 0) {
    assert(*n1 == 5);
    assert(*n2 == 2);
    *err = 1;
    return;
  }

  if (strncmp(name, "INTERVAL", 8) == 0) {
    assert(*n1 == 5);
    assert(*n2 == 2);
    *err = 1;
    return;
  }

  if (strncmp(name, "# STARS", 7) == 0) {
    *value = 1;
    *err = 0;
    return;
  }

  if (strncmp(name, "TIDALUT1", 8) == 0) {
    *value = 1;
    *err = 0;
    return;
  }

  if (strncmp(name, "UTC TAG4", 8) == 0) {
    value[0] = scan_data[scan_nr].year;
    value[1] = scan_data[scan_nr].month;
    value[2] = scan_data[scan_nr].day;
    value[3] = scan_data[scan_nr].hour;
    value[4] = scan_data[scan_nr].min;
    *err = 0;
    return;
  } 

  printf("%s: %.8s(%d, %d, %d)\n", __func__, name, *n1, *n2, *n3);
  *err = 1;
}

void
put4(const char *name, double *value, short *n1, short *n2, short *n3)
{
  if (strncmp(name, "CONSNDEL", 8) == 0) {
    assert (*n1 == 2);
    delay[0] = (value[0] + value[1]) * 1e-6;
    return;
  }

  if (strncmp(name, "CONSNRAT", 8) == 0) {
    assert (*n1 == 1);
    delay[1] = value[0] * 1e-6;
    return;
  }

  if (strncmp(name, "UVW", 3) == 0) {
    assert (*n1 == 3);
    assert (*n2 == 2);
    uvw[0] = value[0];
    uvw[1] = value[1];
    uvw[2] = value[2];
    return;
  }

#if 0
  printf("%s: %.8s(%d, %d, %d)\n", __func__, name, *n1, *n2, *n3);
#endif
}

void
puta(void)
{
  // UNUSED
}

void
putr(void)
{
  // UNUSED
}

void
puti(void)
{
  // UNUSED
}

void
staa(void)
{
  // UNUSED
}

void
stai(void)
{
  // UNUSED
}


//moves to the next record and writes delay to file 
void
mvrec(short *ntoc, short *kmode, short *knum, short *err)
{
  double offset, total_delay, sec_of_day;

  if (!isnan(delay[0])) {
    offset = station_data.clock_early +
      station_data.clock_rate * (scan_data[scan_nr].scan_start
				 + interval * delta_time
				 - station_data.clock_epoch);
    total_delay = delay[0] + offset;
    // We ouput the number of seconds since midnight on the day of the FIRST scan
    if(scan_data[0].day != scan_data[scan_nr].day)
      sec_of_day=scan_data[scan_nr].sec_of_day + (double)24*60*60;
    else
      sec_of_day=scan_data[scan_nr].sec_of_day;

    fwrite(&sec_of_day, 1, sizeof(double), output_file);
    fwrite(uvw, 3, sizeof(double), output_file);
    fwrite(&total_delay, 1, sizeof(double), output_file);

    interval++;
    scan_data[scan_nr].sec = 
      scan_data[scan_nr].sec + delta_time ;
    scan_data[scan_nr].sec_of_day = 
      scan_data[scan_nr].sec_of_day + delta_time ;
  }

  if (interval < scan_data[scan_nr].nr_of_intervals) {
    delay[0] = NAN;
    *err = 0;
    return;
  }

  double empty[] = { 0, 0, 0, 0, 0 };
  fwrite(empty, 5, sizeof(double), output_file);
  delay[0] = NAN;

  *err = 1;
}

void
wridr(void)
{
  // UNUSED
}


extern struct {
  int ILUOUT;
  int KATMC, KATMD, KAXOC, KAXOD, KPTDC, KPTDD, KDNPC, KDNPD;
  int KETDC, KETDD, KIONC, KIOND, KNUTC, KNUTD, KPREC, KPRED;
  int KRELC, KRELD, KSITC, KSITD, KSTRC, KSTRD, KUT1C, KUT1D;
  int KWOBC, KWOBD, KUTCC, KUTCD, KATIC, KATID, KCTIC, KCTID;
  int KPEPC, KPEPD, KDIUC, KDIUD, KM20C, KM20D, KROSC, KROSD;
  int KSTEC, KSTED, KSUNC, KSUND, KSARC, KSARD, KTHEC, KTHED;
  int KMATC, KMATD, KVECC, KVECD, KOCEC, KOCED, KASTC, KASTD;
  int KSTAC, KSTAD, KPLXC, KPLXD, KPANC, KPAND;
} con;


//switches for debug information to stdout
void
con_init(void)
{
  con.ILUOUT = -1;

  con.KATMC=1;
  con.KATMD=0;
  
  con.KAXOC=0;
  con.KAXOD=0;
  
  con.KPTDC=0;
  con.KPTDD=0;
  
  con.KDNPC=0;
  con.KDNPD=0;
  
  con.KETDC=0;
  con.KETDD=0;
  
  con.KIONC=0;
  con.KIOND=0;
  
  con.KNUTC=0;
  con.KNUTD=0;
  
  con.KPREC=0;
  con.KPRED=0;
  
  con.KRELC=0;
  con.KRELD=0;
  
  con.KSITC=0;
  con.KSITD=0;
  
  con.KSTRC=0;
  con.KSTRD=0;
  
  con.KUT1C=0;
  con.KUT1D=0;
  
  con.KWOBC=0;
  con.KWOBD=0;
  
  con.KUTCC=0;
  con.KUTCD=0;
  
  con.KATIC=0;
  con.KATID=0;
  
  con.KCTIC=0;
  con.KCTID=0;
  
  con.KPEPC=0;
  con.KPEPD=0;
  
  con.KDIUC=0;
  con.KDIUD=0;
  
  con.KM20C=0;
  con.KM20D=0;
  
  con.KROSC=0;
  con.KROSD=0;
  
  con.KSTEC=0;
  con.KSTED=0;
  
  con.KSUNC=0;
  con.KSUND=0;

  con.KSARC=0;
  con.KSARD=0;
    
  con.KTHEC=0;
  con.KTHED=0;
  
  con.KMATC=0;
  con.KMATD=0;
  
  con.KVECC=0;
  con.KVECD=0;
  
  con.KOCEC=0;
  con.KOCED=0;
  
  con.KASTC=0;
  con.KASTD=0;
  
  con.KSTAC=0;
  con.KSTAD=0;
  
  con.KPLXC=0;
  con.KPLXD=0;

  con.KPANC=0;
  con.KPAND=0;
}

extern struct {
  char External_inputs[80], Ex_sites[80], Ex_stars[80], Ex_ocean[80];
  char Ex_EOP[80], Ex_tilts[80];
  int External_aprioris, Input_sites, Input_starts, Input_ocean;
  int Input_EOP, Input_tilts;
} extrnl;

void
extrnl_init(void)
{
  extrnl.Input_ocean = 1;
  strncpy(extrnl.Ex_ocean, "./ocean.dat", 80);

  extrnl.Input_tilts = 1;
  strncpy(extrnl.Ex_tilts, "./tilt.dat", 80);
}

void
start(void)
{
  con_init();
  extrnl_init();
}

void
finis(void)
{
}

//below this line developed by Nico Kruithof

void generate_delay_tables(FILE *output, char *stationname) {
  output_file = output;

  assert(stationname[2]=='\0');

  int32_t header_size = 3;
  fwrite(&header_size, 1, sizeof(int32_t), output_file);
  fwrite(stationname, 3, sizeof(char), output_file);

  // scan_nr is a global variable
  for (scan_nr=0; scan_nr<n_scans; scan_nr++) {
    interval = 0;
    calc();
  }
}

int yywrap() {
  return 1;
}
