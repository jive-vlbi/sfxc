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
#include <sys/types.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

char   ctrlFile[252];
char   delaytable[252];
FILE*  deltblP;

float  scan_duration;
int    nr_of_intervals;
int    nr_of_stations;

int    interval;

char Ex_ocean[128];
char Ex_tilts[128];
char *home;

// Station information.
char   site_name[128];
double site_position[3]; // m
int    axis_type; // az : el
double axis_offset; // m
double clock_early; //sec
double clock_rate; //sec/sec 
double clock_epoch; //sec

// Source information.
char   source_name[128];
double ra; // rad
double dec; // rad

// EOP in formation.
double tai_utc; // s
long   eop_ref_epoch; // julian day
int    num_eop_points;
double ut1_utc[10];  // sec
double x_wobble[10]; // asec
double y_wobble[10]; // asec

// Observation time.
int    year;
int    month;
int    day;
int    hour;
int    min;
double sec, sec_, sec_of_day ,delta_time; //unit: seconds
double scan_start;

// Reference frequency.
double freq; // Hz
// Calculated delay.
double delay[2] = { NAN }; //sec

extern void calc(void);

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
    value[1] = axis_offset;
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
    value[3] = site_position[0];
    value[4] = site_position[1];
    value[5] = site_position[2];
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
    value[0] = ra;
    value[1] = dec;
    *err = 0;
    return;
  }

  if (strncmp(name, "FUT1 INF", 8) == 0) {
    assert(*n1 == 4);
    value[0] = eop_ref_epoch;
    value[1] = 1.0;
    value[2] = num_eop_points;
    value[3] = 1.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "FUT1 PTS", 8) == 0) {
    assert(*n1 == num_eop_points);
    for (i = 0; i < num_eop_points; i++)
      value[i] = tai_utc - ut1_utc[i];
    *err = 0;
    return;
  }

  if (strncmp(name, "FWOB INF", 8) == 0) {
    assert(*n1 == 3);
    value[0] = eop_ref_epoch;
    value[1] = 1.0;
    value[2] = num_eop_points;
    *err = 0;
    return;
  }

  if (strncmp(name, "FWOBX&YT", 8) == 0) {
    assert(*n1 == 2);
    assert(*n2 == num_eop_points);
    for (i = 0; i < num_eop_points; i++) {
      value[i * 2] = x_wobble[i] * 1000.;
      value[i * 2 + 1] = y_wobble[i] * 1000.;
    }
    *err = 0;
    return;
  }

  if (strncmp(name, "TAI- UTC", 8) == 0) {
    assert(*n1 == 3);
    value[0] = eop_ref_epoch;
    value[1] = tai_utc;
    value[2] = 0.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "A1 - TAI", 8) == 0) {
    assert(*n1 == 3);
    value[0] = eop_ref_epoch;
    value[1] = 0.0;
    value[2] = 0.0;
    *err = 0;
    return;
  }

  if (strncmp(name, "SEC TAG", 7) == 0) {
    value[0] = sec;
    *err = 0;
    return;
  }

  if (strncmp(name, "REF FREQ", 8) == 0) {
    value[0] = freq;
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
    strncpy(value[1], site_name, sizeof(value[1]));
    *err = 0;
    return;
  }

  if (strncmp(name, "STRNAMES", 8) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 1);
    strncpy(value[0], source_name, sizeof(value[0]));
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
    strncpy(value[1], site_name, sizeof(value[1]));
    *err = 0;
    return;
  }

  if (strncmp(name, "STAR ID", 7) == 0) {
    assert(*n1 == 4);
    assert(*n2 == 1);
    strncpy(value[0], source_name, sizeof(value[0]));
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
    value[1] = axis_type;
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
    value[0] = year;
    value[1] = month;
    value[2] = day;
    value[3] = hour;
    value[4] = min;
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

#if 0
  if (strncmp(name, "UVW", 3) == 0) {
    assert (*n1 == 3);
    assert (*n2 == 2);
    printf("%.8s: (%e, %e, %e) (%e, %e, %e)\n", name, value[0],
        value[1], value[2], value[4], value[5], value[6]);
    return;
  }
#endif

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
void mvrec(short *ntoc, short *kmode, short *knum, short *err)
{

  if (!isnan(delay[0])) {
    //delay[0]=delay, delay[1]=delay_rate
    //delay record for "Huygens type" delay correction
    double offset;
    offset = clock_early + 
      clock_rate*((scan_start+interval*delta_time)-clock_epoch);
    fprintf(deltblP,"%12.6f  %19.16f  %3.1f  %3.1f  \n", 
      sec_of_day, delay[0] + offset, 0.0, 0.0);
    delay[0] = NAN;
  }

  if (interval < nr_of_intervals) {
    *err = 0;
    interval++;
    sec = sec + delta_time ;
    sec_of_day = sec_of_day + delta_time ;
    return;
  }

  *err = 1;
}

void wridr(void)
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
  memcpy(extrnl.Ex_ocean, Ex_ocean, strlen(Ex_ocean));

  extrnl.Input_tilts = 1;
  memcpy(extrnl.Ex_tilts, Ex_tilts, strlen(Ex_tilts));
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


//below this line developed by Ruud Oerlemans

void 
get_eop_data(FILE* ctrlP, int num_eop_points) 
{
  char contents[128],key[64],value[64];

  for (int i=0; i<num_eop_points; i++) {
    fgets(contents,128,ctrlP);
    sscanf(contents,"%s %s\n",key,value);
    ut1_utc[i]=atof(value);        
  }

  for (int i=0; i<num_eop_points; i++) {
    fgets(contents,128,ctrlP);
    sscanf(contents,"%s %s\n",key,value);
    x_wobble[i]=atof(value);        
  }

  for (int i=0; i<num_eop_points; i++) {
    fgets(contents,128,ctrlP);
    sscanf(contents,"%s %s\n",key,value);
    y_wobble[i]=atof(value);        
 }
}



void 
generate_delay_tables(FILE* ctrlP, int nr_of_stations)
{
  char contents[128],key[64],value[64];
  int site_name_len;

  for (int i=0; i<nr_of_stations; i++) {
    //parse the site specific parameters
    while (1){
      if (fgets(contents,128,ctrlP) == (char *) NULL) break;
      if (sscanf(contents,"%s %s\n",key,value) == 2){
        if (!strcmp(key,"DELAYTABLE"))
          strcpy(delaytable,value);
        if (!strcmp(key,"site_name")){
          strcpy(site_name,value);
          site_name_len = strlen(site_name);
          while(site_name_len != 8){
            strcat(site_name," ");
            site_name_len = strlen(site_name);
          }
        }
        if (!strcmp(key,"site_position_x"))
          site_position[0]=atof(value);
        if (!strcmp(key,"site_position_y"))
          site_position[1]=atof(value);
        if (!strcmp(key,"site_position_z"))
          site_position[2]=atof(value);
        if (!strcmp(key,"axis_type"))
          axis_type=atoi(value);
        if (!strcmp(key,"axis_offset"))
          axis_offset=atof(value);
        if (!strcmp(key,"clock_early")) 
          clock_early=atof(value)/1000000.;
        if (!strcmp(key,"clock_rate")) 
          clock_rate=atof(value)/1000000.;
        if (!strcmp(key,"clock_epoch")) {
          clock_epoch=atof(value);
          break;
        }
      }
    }


    printf("\n\nsite_name          %s\n",site_name);
    printf("site_position_x    %f\n",site_position[0]);
    printf("site_position_y    %f\n",site_position[1]);
    printf("site_position_z    %f\n",site_position[2]);
    printf("axis_type          %d\n",axis_type);
    printf("axis_offset        %f\n",axis_offset);
    printf("clock_early        %g\n",clock_early);
    printf("clock_rate         %g\n",clock_rate);
    printf("clock_epoch        %f\n",clock_epoch);
    printf("\ncreating delaytable: %s\n\n",delaytable);
    //open the delay table file
    deltblP = fopen(delaytable,"w");
    //reset the next parameters:
    //- sec
    //- the seconds since start of the day 
    //- interval counter for every site
    sec=sec_ ;//calc works with this one
    sec_of_day = hour*3600. + min*60. + sec; //delay table for sfxc needs this one
    interval=0;
    //generate the delay tables 
    calc();
    //close the delay table file
    fclose(deltblP);
  }

}



int 
parse_control_file(char *ctrlFile)
{
  FILE *ctrlP;
  char errMessage[256],contents[128],key[64],value[64];
  
  ctrlP = fopen(ctrlFile,"r");
  if (ctrlP == (FILE*) NULL)
  {
    strcpy(errMessage,"parse_control_file(), fopen(), ");
    strcat(errMessage,ctrlFile);
    perror(errMessage);
    return 1;
  }


  while (1) {
    if (fgets(contents,128,ctrlP) == (char *) NULL) break;
    if (sscanf(contents,"%s %s\n",key,value) == 2){
      
      if (!strcmp(key,"scan_duration"))
        scan_duration = atof(value);
    
      // Source information.
      if (!strcmp(key,"source_name"))
        strcpy (source_name,value);
      if (!strcmp(key,"source_ra"))
        ra = atof(value);//rad
      if (!strcmp(key,"source_dec"))
        dec = atof(value);//rad
        
      // EOP in formation.
      if (!strcmp(key,"tai_utc"))
        tai_utc = atof(value);
      if (!strcmp(key,"eop_ref_epoch"))
        eop_ref_epoch = strtol(value, NULL, 10);
      if (!strcmp(key,"num_eop_points")){
        num_eop_points = atoi(value);
        get_eop_data(ctrlP, num_eop_points);
      }

      // Observation time.
      if (!strcmp(key,"year"))
        year = atoi(value);
      if (!strcmp(key,"month"))
        month = atoi(value);
      if (!strcmp(key,"day"))
        day = atoi(value);
      if (!strcmp(key,"hr"))
        hour = atoi(value);
      if (!strcmp(key,"min"))
        min = atoi(value);
      if (!strcmp(key,"sec"))
        sec_ = atof(value);
      if (!strcmp(key,"scan_start"))
        scan_start = atof(value);

      // Reference frequency.
      if (!strcmp(key,"freq")){
        freq = atof(value);
      }

      // nr of stations/sites
      if (!strcmp(key,"nr_of_stations")){

        printf("source_name                  %s\n",source_name);
        printf("source_ra                    %f\n",ra);
        printf("source_dec                   %f\n",dec);
        printf("\ntai_utc                      %f\n",tai_utc);
        printf("eop_ref_epoch                %ld\n",eop_ref_epoch);
        printf("num_eop_points               %d\n",num_eop_points);
        printf("ut1-utc                      ");
        for(int i=0; i<num_eop_points; i++) 
          printf("%f ",ut1_utc[i]);
        printf("\n");
        printf("x_wobble                     ");
        for(int i=0; i<num_eop_points; i++) 
          printf("%f ",x_wobble[i]);
        printf("\n");
        printf("y_wobble                     ");
        for(int i=0; i<num_eop_points; i++) 
          printf("%f ",y_wobble[i]);
        printf("\n");
        printf("\nyear:month:day:hour:min:sec  %d:%d:%d:%d:%d:%f\n",year,month,day,hour,min,sec_);
        printf("scan_start                   %f\n",scan_start);
        printf("scan_duration                %f\n",scan_duration);
        printf("frequency                    %f\n",freq);
      
        nr_of_stations = atoi(value);
        nr_of_intervals=scan_duration/delta_time+1;
        // parse station information and generate the delay table
        generate_delay_tables(ctrlP,nr_of_stations);
      }
      
      
    }
  }
  
  //close control file
  fclose(ctrlP);
  
  return 0;
}


//
int 
main(int argc, char *argv[])
{
  if (argc!=3) {
    printf("\nUsage: delmo  delmo_control_file  time_interval\n\n");
    printf("delmo_control_file : generated automatically by application vex2ccf\n");
    printf("dependencies       : files ocean.dat, tilt.dat and DE405_le.jplshould be in $HOME/bin.\n");
    return 0;
  }
  
  strcpy(ctrlFile,argv[1]);//control file name
  delta_time=atof(argv[2]);//time between consecutive delay table records

  //set some file names. calc relies on it.
  home = getenv("HOME");
  strcpy(Ex_ocean,home);
  strcat(Ex_ocean,"/bin/ocean.dat");
  strcpy(Ex_tilts,home);
  strcat(Ex_tilts,"/bin/tilt.dat");

  //make a local copy of DE405_le.jpl. calc relies on it.
  char cmd[256];
  strcpy(cmd, "cp ");
  strcat(cmd, home);
  strcat(cmd, "/bin/DE405_le.jpl .");
  system (cmd);

  //parse delmo control file and process it
  if ( parse_control_file(ctrlFile)!=0) return 1;

/*   //remove local copy of DE405_le.jpl */
/*   strcpy(cmd, "rm -f "); */
/*   strcat(cmd, "DE405_le.jpl"); */
/*   system (cmd); */

  return EXIT_SUCCESS;
}
