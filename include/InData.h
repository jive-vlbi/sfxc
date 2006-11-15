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
#include <Input_reader.h>
#include <vector>


int FindOffsets(std::vector<Input_reader *> input_readers,
                int Ncores);

int fill_Mk4frame(int station, Input_reader &reader, double *Mk4frame,
                  double *signST, double *magnST, INT64 *Nsamp);

INT64 Delaydt(char *DelayTableName);

int ReadDelayTable(char *DelayTableName, INT64& tableStartTime, INT64 delaydt,
    int Ndr, int Cde, int Mde, int Rde,
    INT64 *tdel, double *cdel, double *mdel, double *rdel, double *fdel);

double  ParInteRp(double Time, double StartTime, double *Y, double dT, INT64 cpMax);

#endif // INDATA_H
