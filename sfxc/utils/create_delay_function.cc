/*
$Author$
$Date$
$Name$
$Revision$
$Source$

  This small utility reads data from a file (argv[1]), interpolates
  the data and writes it to file (argv[2]).

  You will need gsl (GNU scientific library) for the interpolation.

*/

#include <assert.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>

#define N 25

int
main (int argc, char *argv[])
{
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <infile> <outfile>" << std::endl;
  }

  std::ifstream infile(argv[1]);
  assert(infile.is_open());
  FILE *outfile = fopen(argv[2], "w");
  assert(outfile != 0);

  double datax[25], datay[25];

  int rows = 0;
  // read data: (9 columns for n05c2)
  int c1;
  double c2,c3,c4,c5,c6,c7,c8,c9;
  while (infile >> c1 >> c2 >> c3 >> c4 >> c5 >> c6 >> c7 >> c8 >> c9) {
    assert(rows <= N);
    // c1 is hhmmss format, get the number of seconds from 24:00h:
    int sec = c1%100; // Seconds
    sec += ((c1/100)%100)*60; // Minutes
    sec += ((c1/10000)%100)*60*60; // Hours
    
    datax[rows] = sec;
    datay[rows] = c8;

    // Print out data for debugging purposes:
    printf ("%d %20.16f\n", sec, datay[rows]);

    rows++;
  }

  { // Do the interpolation
    gsl_interp_accel *acc 
      = gsl_interp_accel_alloc ();
    gsl_spline *spline 
      = gsl_spline_alloc (gsl_interp_cspline, rows);
    
    gsl_spline_init (spline, datax, datay, rows);
    
    for (double xi = datax[0]-3; xi < datax[rows-1]+3; xi += 0.125) {
      double yi = gsl_spline_eval (spline, xi, acc);
      
      // Print with high precision:
      fprintf (outfile, "%f %20.16f 0.0 0.0\n", xi, yi/1000000);
      //outfile << xi << " " << yi << " 0.0 0.0" << std::endl;
    }

    gsl_spline_free (spline);
    gsl_interp_accel_free(acc);
  }
  return 0;
}

