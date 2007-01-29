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
#include "Data_writer.h"




int CorrelateBufs(std::vector<Data_reader *> &readers);
int CorrelateBufs_initialise_time_slice();
int CorrelateBufs_initialise_correlator(std::vector<Data_reader *> &readers_);
int CorrelateBufs_process_segment();
int CorrelateBufs_finalise();


// NGHK: Defined in ProcessData.cc, remove as soon as there is a correlate class
void correlation_add_delay_table(DelayTable &table);

void set_log_writer(Log_writer &writer);
Log_writer &get_log_writer();
  
void set_data_writer(Data_writer &writer);
Data_writer &get_data_writer();
