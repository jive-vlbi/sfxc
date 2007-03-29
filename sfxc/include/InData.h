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

#include <Data_reader.h>
#include <vector>

void set_log_writer(Log_writer &log_writer_);

Log_writer &get_log_writer();

int FindOffsets(std::vector<Data_reader *> input_readers,
                int Numtask, int rank);
                
int show_MK4_headers(std::vector<Data_reader *> input_readers);

int fill_Mk4frame(int station, Data_reader &reader, double **Mk4frame,
                  double *signST, double *magnST, INT64 *Nsamp);

int fill_Mk4frame(int sn, Data_reader &reader, double **Mk4frame);

int FindHeaderMk4(Data_reader &reader, int station, int& jsynch,
  INT64& usTime, INT64 usStart);

#endif // INDATA_H
