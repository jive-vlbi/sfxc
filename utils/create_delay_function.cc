/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 * 
 * This small utility reads data from a file, interpolates
 * the data and writes it to file.
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

void usage() {
  std::cout << "Usage: create_delay_function " 
            << "<nCols> <selectedCol> <infile> <outfile>" << std::endl;
}

int
main (int argc, char *argv[])
{
  if (argc != 5) {
    usage();
    exit(1);
  }

  int nColumns, data_column;
  if (sscanf(argv[1], "%d", &nColumns) ==0 ) {
    usage();
    return 1;
  }
  if (sscanf(argv[2], "%d", &data_column) ==0 ) {
    usage();
    return 1;
  }
  if (data_column >= nColumns) {
    std::cout << "selectedColumn is not smaller nCols: "
              << data_column << " >= " << nColumns << std::endl;
    return 1;
  }

  // The first column (time) is stored separately
  data_column--;

  std::ifstream infile(argv[3]);
  assert(infile.is_open());
  FILE *outfile = fopen(argv[4], "w");
  assert(outfile != 0);

  // Do the actual work:

  std::cout.precision(20);

  std::vector<double> times, delays;

  // Read the data:
  int time;
  double c[nColumns-1];
  while (infile >> time) {
    for (int i=0; i<nColumns-1; i++) {
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
    // c[0] is the second column in the data file (first is the time)
    // c[1] is the third, etc.
    delays.push_back(c[data_column]);

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
      // xi is time in second miliseconds from the beginning of the day
      // yi is delay in microseconds
      fprintf (outfile, "%d %20.16f 0.0 0.0\n", int(xi), yi);
    }

    gsl_spline_free (spline);
    gsl_interp_accel_free(acc);
  }
  return 0;
}

