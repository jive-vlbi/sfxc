/*
  purpose    : delmo generates delay tables in sfxc type format according to 
  the parameters set in a delmo_control_file, which was generated 
  with the utility vex2ccf

  last change: 29-05-2007
  authors    : RHJ Oerlemans, M Kettenis

  dependencies: files ocean.dat tilt.dat and DE405_le.jpl should be in $HOME/bin.


  TODO RHJO
  1) eop_ref_epoch (Julian Day) in double or long?

*/

#include <generate_delay_model.h>

#include <iostream>
#include <string>
#include <assert.h>
#include "vexplus.h"
#include <utils.h>
#include "stdio.h"

extern "C" void generate_delay_tables(FILE *output);

// Time between sample points
const double delta_time = 1.; // in seconds

// Data needed to generate the delay table
struct Station_data station_data;
int                 n_scans;
struct Scan_data    *scan_data;

// Reads the data from the vex-file
int initialise_data(const char *vex_file,
                    const char *station_name);

int 
main(int argc, char *argv[]) {
  if (argc!=4) {
    printf("\n");
    printf("Usage: generate_delay_model <vexfile> <stationname> <outputfile>\n");
    printf("dependencies       : CALC_DIR contains ocean.dat, tilt.dat ");
    printf(                     "and DE405_le.jpl.\n\n");
    return 0;
  }

  // Read the vex-file
  int err;
  err = initialise_data(argv[1], argv[2]);
  if (err != 0) {
    std::cout << "Could not initialise the delay model" << std::endl;
    exit(1);
  }

  // Open the output file
  FILE *output_file = fopen(argv[3], "w");
  assert(output_file!=NULL);

  // Change to the CALC-directory
  // Goto the location of calc-10 files ocean.dat, tilt.dat and DE405_le.jpl
  char *dir = getenv("CALC_DIR");
  if (dir != NULL) {
    int err = chdir(dir);
    assert(err == 0);
  }
  
  // call the c-function that calls the FORTRAN calc code
  generate_delay_tables(output_file);

  return EXIT_SUCCESS;
}

/*******************************************/
/**  Helping functions                    **/
/*******************************************/


long str_to_long (std::string inString, int pos, int length)
{
  std::string str=inString.substr(pos,length);
  char tmp[length+1];
  strcpy(tmp,str.c_str());
  
  char *endp;
  long sval = strtol(tmp, &endp, 10);
  if (endp == tmp) {
    fprintf(stderr,"**** Unable to convert string %s into long\n",tmp);
    return -1;
  } else {
    return sval;
  }  
}

bool leap_year(int year) {
  return ( (year%4==0) && ((year%100!=0) || (year%400==0)) ? 1 : 0);
}

//input  year, day of year
//output month, day of month
void yd2md(int year, int doy, int &month, int &dom){

  const int monthdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int rest_of_days=doy;
  int current_month=1;
  int length_current_month = monthdays[current_month-1];
  while(rest_of_days > length_current_month) {
    rest_of_days -= length_current_month;
    current_month++;
    length_current_month =
      (monthdays[current_month-1] + 
     (current_month==2 && leap_year(year) ? 1 : 0));
  }
  dom = rest_of_days;
  month = current_month;
}

//input year month day
//output Julian Day
long long JD(int y, int m, int d)
{
   return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
          ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
          ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
          d - 32075;  
}

//returns clock epoch in seconds for timeString
//assumption: |year-ref_year|<=1
long ceps(std::string timeString, int ref_year)
{
  long clock_epoch=0;
  int year = str_to_long(timeString,0,4);  //pos=0, length=4
  int doy = str_to_long(timeString,5,3);
  int hr = str_to_long(timeString,9,2);
  int min = str_to_long(timeString,12,2);
  int sec = str_to_long(timeString,15,2);
  assert(abs(year-ref_year)<=1);
  if (ref_year == year) {
    clock_epoch = sec + 60*min + 3600*hr + 86400*doy;
  } else {
    int days_per_year = (leap_year(ref_year) ? 366 : 365);
    clock_epoch = sec + 60*min + 3600*hr + 86400*doy +
                  (year-ref_year)*days_per_year * 86400;
  }
  return clock_epoch;
}



int initialise_data(const char *vex_filename, 
                    const char *station_name) {
  // Vex file
  VexPlus vex_file = VexPlus(vex_filename);
  vex_file.parseVex();
  
  // get station number
  int station_nr=-1;
  for (int i=0; i<vex_file.N_Stations(); i++) {
    if (station_name == vex_file.Station(i)) {
      station_nr = i;
    }
  }
  assert(station_nr >= 0);

  { // Station data
    // Station related data
    strcpy(station_data.site_name,
           vex_file.Site(station_name).c_str());
    for (int i=vex_file.Site(station_name).size(); i<8; i++) {
      station_data.site_name[i] = ' ';
    }
    station_data.site_position[0] = vex_file.SiteX(station_name);
    station_data.site_position[1] = vex_file.SiteY(station_name);
    station_data.site_position[2] = vex_file.SiteZ(station_name);
    if (vex_file.AxisMount(station_name) == "az_el") 
      station_data.axis_type=3;
    if (vex_file.AxisMount(station_name) == "ha_dec") 
      station_data.axis_type=4;
    station_data.axis_offset = vex_file.AxisOffset(station_name);
    station_data.clock_early =vex_file.ClockOffset(station_name)/1000000.;
    station_data.clock_rate = vex_file.ClockRate(station_name)/1000000.;
    {
      std::string clock_epoch = vex_file.ClockEpoch(station_name);
      std::string startTime=vex_file.ScanStart(station_name, /*scan_nr*/0);
      int year_start = str_to_long(startTime,0,4);  //pos=0, length=4;
      station_data.clock_epoch = ceps(clock_epoch,year_start);
    }
    { // EOP information
      station_data.tai_utc = vex_file.TAI_UTC();
      std::string eop_ref_epoch=vex_file.EOPEpoch();
      int year = str_to_long(eop_ref_epoch,0,4);  //pos=0, length=4
      int doy = str_to_long(eop_ref_epoch,5,3);
      int month, day;
      yd2md(year,doy,month,day);
      station_data.eop_ref_epoch = JD(year,month,day); // Julian day
      station_data.num_eop_points = vex_file.N_EOP_Points();
      assert(vex_file.N_EOP_Points() <= 10);
      for(int i=0; i<vex_file.N_EOP_Points(); i++) 
        station_data.ut1_utc[i] = vex_file.UT1_UTC(i);
      for(int i=0; i<vex_file.N_EOP_Points(); i++) 
        station_data.x_wobble[i] = vex_file.XWobble(i);
      for(int i=0; i<vex_file.N_EOP_Points(); i++) 
        station_data.y_wobble[i] = vex_file.YWobble(i);
    }
  }

  // Scan related data
  n_scans = vex_file.N_Scans(station_name);
  scan_data = new struct Scan_data[n_scans];
  for (int scan_nr=0; scan_nr< n_scans; scan_nr++) {
    struct Scan_data &scan = scan_data[scan_nr];

    std::string startTime = vex_file.ScanStart(station_name, scan_nr);
    scan.year = str_to_long(startTime,0,4);  //pos=0, length=4
    int doy = str_to_long(startTime,5,3);
    // convert day of year to (month,day)
    yd2md(scan.year,doy,scan.month,scan.day); 
    scan.hour  = str_to_long(startTime,9,2);
    scan.min = str_to_long(startTime,12,2);
    scan.sec = str_to_long(startTime,15,2);
    // delay table for sfxc needs this one
    scan.sec_of_day = scan.hour*3600. + scan.min*60. + scan.sec; 
    scan.scan_start = 
      scan.sec + 60*(scan.min + 60*(scan.hour + 24*(double)doy));
    double duration = vex_file.ScanDuration(station_name, scan_nr);
    scan.scan_stop = scan.scan_start + duration;
    scan.nr_of_intervals = (int)(duration/delta_time)+1;

    // Source information.
    // find source
    int source_nr= -1;
    for(int source_it=0; source_it<vex_file.N_Sources(); source_it++) {
      assert(scan_nr >= 0);
      if(vex_file.SourceName(source_it) == 
         vex_file.ScanSource(station_name,scan_nr)) {
        source_nr = source_it;
      }
    }
    assert(source_nr >= 0);
    strncpy(scan.source_name, 
            vex_file.SourceName(source_nr).c_str(),
            80);
    for (int i=vex_file.SourceName(source_nr).size(); i<80; i++) {
      scan.source_name[i] = ' ';
    }
    scan.ra  = vex_file.Source_RA(source_nr);
    scan.dec = vex_file.Source_Dec(source_nr);
    
    if (scan_nr > 0) {
      assert (scan_data[scan_nr-1].scan_start + 
              (scan_data[scan_nr-1].nr_of_intervals-1)*delta_time <= 
              scan.scan_start);
      assert (scan_data[scan_nr-1].scan_stop <= scan.scan_start);
    }
  }

  return 0;
}
