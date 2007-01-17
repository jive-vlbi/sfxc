/*
  CVS keywords
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#include "Data_reader.h"
#include "delayTable.h"
#include "Log_writer.h"

void correlation_add_delay_table(DelayTable &table);
void set_log_writer(Log_writer &writer);

int CorrelateBufs(int core, std::vector<Data_reader *> &readers);

// NGHK: Defined in ProcessData.cc, remove as soon as there is a correlate class
extern Log_writer &log_writer;

  

