/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 * 
 * This small utility reads data from a file (argv[1]), interpolates
 * the data and writes it to file (argv[2]).
 * 
 * You will need gsl (GNU scientific library) for the interpolation.
*/

#include <assert.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <vector>

#define NCOLS 9

int
main (int argc, char *argv[])
{
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <infile> <outfile>" << std::endl;
    exit(1);
  }

  std::ifstream infile(argv[1]);
  assert(infile.is_open());
  FILE *outfile = fopen(argv[2], "w");
  assert(outfile != 0);

  std::cout.precision(20);

  std::vector<double> times, delays;

  // read data: (9 columns for n05c2)
  int time;
  double c[NCOLS-1];
  while (infile >> time) {
    for (int i=0; i<NCOLS-1; i++) {
      if (!(infile >> c[i])) {
        std::cout << "Incomplete line" << std::endl;
        break;
      }
    }
    // c1 is hhmmss format, get the number of seconds from 24:00h:
    int milisec = time%100; // Seconds
    milisec += ((time/100)%100)*60; // Minutes
    milisec += ((time/10000)%100)*60*60; // Hours
    milisec *= 1000;
    
    times.push_back(milisec);
    delays.push_back(c[0]);

    // Print out data for debugging purposes:
    std::cout << times.back() << " " <<  delays.back() << std::endl;
  }

  { // Do the interpolation
    gsl_interp_accel *acc 
      = gsl_interp_accel_alloc ();
    gsl_spline *spline 
      = gsl_spline_alloc (gsl_interp_cspline, times.size());
    
    gsl_spline_init (spline, &(times[0]), &(delays[0]), times.size());
    
    // add 3 seconds:
    for (double xi = times[0]-3000; xi <= times.back()+3000; xi += 125) {
      double yi = gsl_spline_eval (spline, xi, acc);
      // Print with high precision:
      fprintf (outfile, "%d %20.16f 0.0 0.0\n", 
               int(xi), 
               (yi)/1000000 - 0./16000000);
    }

    gsl_spline_free (spline);
    gsl_interp_accel_free(acc);
  }
  return 0;
}

