#include "gen_defines.h"

int FindOffsets(void);

int fill_Mk4frame(int sn, int inFile, double *Mk4frame,
  double *signST, double *magnST, INT64 *Nsamp);

INT64 Delaydt(char *DelayTableName);

int ReadDelayTable(char *DelayTableName, INT64 tableStartTime,
    int Ndr, int Cde, int Mde, int Rde,
    INT64 *tdel, double *cdel, double *mdel, double *rdel, double *fdel);

double  ParInteRp(double Time, double StartTime, double *Y, double dT, INT64 cpMax);
