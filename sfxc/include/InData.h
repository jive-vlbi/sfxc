/*
  CVS keywords
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef INDATA_H
#define INDATA_H

#include "gen_defines.h"
#include <Data_reader.h>
#include <vector>


int FindOffsets(std::vector<Data_reader *> input_readers,
                int Numtask, int rank);

int fill_Mk4frame(int station, Data_reader &reader, double **Mk4frame,
                  double *signST, double *magnST, INT64 *Nsamp);


#endif // INDATA_H
