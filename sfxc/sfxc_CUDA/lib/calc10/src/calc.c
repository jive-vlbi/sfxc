#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "constants.h"

const char *Ex_ocean = "ocean.dat";
const char *Ex_tilts = "tilt.dat";

// Station information.
char   site_name[128];
double site_position[3]; // m
int    axis_type; // az : el
double axis_offset; // m

/* Source information. */
char   source_name[128];
double ra; /* rad */
double dec; /* rad */

// EOP in formation.
double tai_utc; // s
double eop_ref_epoch; // julian day
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
double sec;

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
	/* UNUSED */
}

void
adda(void)
{
	/* UNUSED */
}

void
addi(void)
{
	/* UNUSED */
}

void
addr(void)
{
	/* UNUSED */
}

void
dela(void)
{
	/* UNUSED */
}

void
delr(void)
{
	/* UNUSED */
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
		strncpy(value[0], "UTC    ", 8);
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
	/* UNUSED */
}

void
putr(void)
{
	/* UNUSED */
}

void
puti(void)
{
	/* UNUSED */
}

void
staa(void)
{
	/* UNUSED */
}

void
stai(void)
{
	/* UNUSED */
}

void
mvrec(short *ntoc, short *kmode, short *knum, short *err)
{
	static int n = 0;

	if (!isnan(delay[0])) {
		printf("%e %e\n", delay[0], delay[1]);
		delay[0] = NAN;
	}

	if (n < 180) {
		*err = 0;
		n++;
		sec++;
		return;
	}

	*err = 1;
}

void
wridr(void)
{
	/* UNUSED */
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

int
main(void)
{
  // Station information.
  strcpy(site_name,"MEDICINA");
  site_position[0] = 4461370.000000;
  site_position[1] =  919597.000000;
  site_position[2] = 4449560.000000;
  axis_type = 3; /* az : el */
  axis_offset = 1.827300; /* m */

  // Source information.
  strcpy (source_name,"DA193");
  ra  = 1.551220; // rad 
  dec = 0.694879; // rad

  // EOP in formation.
  tai_utc = 33.0; // s
  eop_ref_epoch = 2453903; // julian day 
  num_eop_points = 3;
  ut1_utc[0] = 0.2006443;
  ut1_utc[1] = 0.2001595;
  ut1_utc[2] = 0.1994164;
  x_wobble[0] = 0.126003;
  x_wobble[1] = 0.126181;
  x_wobble[2] = 0.126336;
  y_wobble[0] = 0.316220;
  y_wobble[1] = 0.315367;
  y_wobble[2] = 0.314334;

  // Observation time.
  year = 2006;
  month = 6;
  day = 17;
  hour = 7;
  min = 32;
  sec = 0;

  // Reference frequency.
  freq = 4974.49e6; // Hz

  //calculate the delays and delay rates
  calc();

  return EXIT_SUCCESS;
}
