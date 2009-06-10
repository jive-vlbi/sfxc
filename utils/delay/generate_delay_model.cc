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

#include <iostream>
#include <cstring>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <math.h>

#include "generate_delay_model.h"
#include "vex/Vex++.h"

#define PI (3.14159265358979323846) //copied from prep_job/calc_model.h
#define SPEED_OF_LIGHT (299792458.0)
#define SECS_PER_DAY (86400)
#define IPS_FEET (12)

extern "C" void generate_delay_tables(FILE *output, char *stationname);

// Time between sample points
const double delta_time = 1; // in seconds

// Data needed to generate the delay table
struct Station_data station_data;
int                 n_scans;
struct Scan_data    *scan_data;

// Reads the data from the vex-file
int initialise_data(const char *vex_file,
                    const std::string &station_name);

int
main(int argc, char *argv[]) {
  if (argc!=4) {
    printf("\n");
    printf("Usage: generate_delay_model <vexfile> <stationname> <outputfile>\n");
    printf("dependencies       : CALC_DIR contains ocean.dat, tilt.dat ");
    printf(                     "and DE405_le.jpl.\n\n");
    return 0;
  }

  //Read the vex-file
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
  generate_delay_tables(output_file, argv[2]);

  return EXIT_SUCCESS;
}

/*******************************************/
/**  Helping functions                    **/
/*******************************************/


long str_to_long (std::string inString, int pos, int length) {
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
void yd2md(int year, int doy, int &month, int &dom) {

  const int monthdays[] = {
                            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
                          };
  int rest_of_days=doy;
  int current_month=1;
  int length_current_month = monthdays[current_month-1];
  while (rest_of_days > length_current_month) {
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
long long JD(int y, int m, int d) {
  return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
         ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
         ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
         d - 32075;
}

//returns clock epoch in seconds for timeString
//assumption: |year-ref_year|<=1
long ceps(std::string timeString, int ref_year) {
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
/*
  double site_position(const Vexpp_node &vex,
  const std::string &station) {
  std::vector<double> result;
  Vex::Node::const_iterator position =
  vex["SITE"][vex["STATION"][station]["SITE"]->to_string()]["site_position"];
  int i=0;
  for (Vex::Node::const_iterator site = position->begin();
  site != position->end(); ++site) {
  double pos;
  int err = sscanf(site->to_string().c_str(), "%lf m", &pos);
  assert(err == 1);
  result[i] = pos;
  i++;
  }
  }
*/
int initialise_data(const char *vex_filename,
                    const std::string &station_name) {
  Vex vex(vex_filename);

  Vex::Node root = vex.get_root_node();

  std::string site_name =
    root["STATION"][station_name]["SITE"]->to_string();

  strcpy(station_data.site_name, site_name.c_str());
  for (int i=site_name.size(); i<8; i++) {
    station_data.site_name[i] = ' ';
  }

  station_data.site_name[8]='\0';

  int i = 0;
  Vex::Node::const_iterator position =
    vex.get_root_node()["SITE"][site_name]["site_position"];
  for (Vex::Node::const_iterator site = position->begin();
       site != position->end(); ++site) {
    double pos;
    int err = sscanf(site->to_string().c_str(), "%lf m", &pos);
    assert(err == 1);
    station_data.site_position[i] = pos;
    i++;
  }

  if (vex.get_root_node()["ANTENNA"][site_name]["axis_type"][0]->to_string()=="az")
    station_data.axis_type=3;
  if (vex.get_root_node()["ANTENNA"][site_name]["axis_type"][0]->to_string()=="ha")
    station_data.axis_type=1;

  station_data.axis_offset =
    vex.get_root_node()["ANTENNA"][site_name]["axis_offset"]->to_double_amount("m");

  const std::string site_clock =
    vex.get_root_node()["STATION"][station_name]["CLOCK"]->to_string();

  station_data.clock_early =
    vex.get_root_node()["CLOCK"][site_clock]["clock_early"][1]->to_double()/1000000.;
  station_data.clock_rate =
    vex.get_root_node()["CLOCK"][site_clock]["clock_early"][3]->to_double()/1000000.;

  {
    std::string clock_epoch = vex.get_root_node()["CLOCK"][site_clock]["clock_early"][2]->to_string();
    std::string startTime = vex.get_root_node()["SCHED"]->begin()["start"]->to_string();
    int year_start = str_to_long(startTime,0,4);  //pos=0, length=4;
    station_data.clock_epoch = ceps(clock_epoch,year_start);
  }

  { // EOP information
    for (Vex::Node::const_iterator eop = vex.get_root_node()["EOP"]->begin();
         eop != vex.get_root_node()["EOP"]->end(); ++eop) {
      station_data.tai_utc = eop["TAI-UTC"]->to_double();
      std::string eop_ref_epoch = eop["eop_ref_epoch"]->to_string();
      int year = str_to_long(eop_ref_epoch,0,4);  //pos=0, length=4
      int doy = str_to_long(eop_ref_epoch,5,3);
      int month, day;
      yd2md(year,doy,month,day);
      double hour = str_to_long(eop_ref_epoch,9,2);
      station_data.eop_ref_epoch = JD(year,month,day) + (hour - 12) / 24; // Julian day
      station_data.num_eop_points = eop["num_eop_points"]->to_int();
      assert(station_data.num_eop_points<=10);
      for (int i=0; i<station_data.num_eop_points; i++) {
        station_data.ut1_utc[i] = eop["ut1-utc"][i]->to_double_amount("sec");
        station_data.x_wobble[i] = eop["x_wobble"][i]->to_double_amount("asec");
        station_data.y_wobble[i] = eop["y_wobble"][i]->to_double_amount("asec");
      }
    }
  }

  // Scan related data

  n_scans = 0;
  for (Vex::Node::const_iterator scan = vex.get_root_node()["SCHED"]->begin();
       scan != vex.get_root_node()["SCHED"]->end(); ++scan) {
    for (Vex::Node::const_iterator scan_it = scan->begin("station");
         scan_it != scan->end("station"); ++scan_it) {
      if (scan_it[0]->to_string() == station_name) {
        n_scans +=1;
      }
    }
  }

  std::string startTime;
  scan_data = new struct Scan_data[n_scans];
  int scan_nr=0;
  for (Vex::Node::const_iterator scan_block = vex.get_root_node()["SCHED"]->begin();
       scan_block != vex.get_root_node()["SCHED"]->end(); ++scan_block) {
    for (Vex::Node::const_iterator scan_it = scan_block->begin("station");
         scan_it != scan_block->end("station"); ++scan_it) {
      if (scan_it[0]->to_string() == station_name) {
        assert(scan_nr < n_scans);
        struct Scan_data &scan = scan_data[scan_nr];

        startTime = scan_block["start"]->to_string();
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
        double duration = scan_it[2]->to_double_amount("sec");
        scan.scan_stop = scan.scan_start + duration;
        scan.nr_of_intervals = (int)(duration/delta_time);
        for (Vex::Node::const_iterator source_block =
               vex.get_root_node()["SOURCE"]->begin(scan_block["source"]->to_string());
             source_block != vex.get_root_node()["SOURCE"]->end(scan_block["source"]->to_string());
             ++source_block) {

          strncpy(scan.source_name,
                  scan_block["source"]->to_string().c_str(),
                  80);
          for (int i=strlen(scan_block["source"]->to_string().c_str()); i<80; i++) {
            scan.source_name[i] = ' ';
          }
          scan.source_name[80]='\0';

          int   hours, minutes, degs;
          double seconds;
          sscanf( source_block["ra"]->to_string().c_str(), "%dh%dm%lfs", &hours, &minutes, &seconds );
          scan.ra = ((hours*3600 + 60*minutes + seconds ) * 2 * PI)/SECS_PER_DAY;
          sscanf( source_block["dec"]->to_string().c_str(), "%dd%d\'%lf\"", &degs, &minutes, &seconds );
          if (strchr(source_block["dec"]->to_string().c_str(), '-'))
            scan.dec = -1*(PI/180)*(abs(degs)+minutes/60.0+seconds/3600);
          else
            scan.dec = (PI/180)*(abs(degs)+minutes/60.0+seconds/3600);
        }
        scan_nr +=1;
      }
    }
  }
  return 0;
}
