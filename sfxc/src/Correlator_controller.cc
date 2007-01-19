#include <Correlator_controller.h>
#include <signal.h>
#include <iostream>
#include <ProcessData.h>
#include <InData.h>

#include <Data_reader_file.h>
#include <utils.h>

#include <assert.h>

/// TODO: NGHK: REMOVE
#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
extern RunP  RunPrms;
extern GenP  GenPrms;
extern StaP  StaPrms[NstationsMax];
extern INT64 sliceStartByte[NstationsMax][NprocessesMax];
extern INT64 sliceStartTime [NprocessesMax];
extern INT64 sliceStopTime  [NprocessesMax];
extern INT64 sliceTime;

#include <MPI_Transfer.h>

Correlator_controller::Correlator_controller(Buffer<output_value_type> &output_buffer,
                                           Log_writer &log_writer)
 : Controller(log_writer), 
   output_buffer(output_buffer), running(false), curr_station(0)
{
}

Correlator_controller::~Correlator_controller()
{
}

Controller::Process_event_status 
Correlator_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_STATION_NUMBER:
    {
      log_writer.MPI(2,"MPI_TAG_SET_STATION_NUMBER");
      int station;
      MPI_Recv(&station, 1, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      curr_station = station;
      if (data_readers.capacity() <= static_cast<size_t>(curr_station)) {
        data_readers.reserve(curr_station+1);
      }
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_INPUT_NODE_FILE:
    {
      log_writer.MPI(2,"MPI_TAG_SET_INPUT_NODE_FILE");
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      Data_reader *reader = new Data_reader_file(filename);
      data_readers.push_back(reader);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_OUTPUT_NODE_FILE:
  {
    log_writer.MPI(2,"MPI_TAG_SET_OUTPUT_NODE_FILE");
    int size;
    MPI_Get_elements(&status, MPI_CHAR, &size);
    assert(size > 0);
    char filename[size];
    MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);

    GenPrms.set_corfile(filename);

    return PROCESS_EVENT_STATUS_SUCCEEDED;
  }
  case MPI_TAG_SET_CONTROL_FILE:
    {
      log_writer.MPI(2,"MPI_TAG_SET_CONTROL_FILE: DEPRECATED");
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      if (initialise_control(filename, log_writer) != 0) {
        log_writer.MPI(2,"Initialisation using control file failed");
      }

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_START_TIME:
    {
      log_writer.MPI(2,"MPI_TAG_SET_START_TIME");
      int time[5];
      MPI_Recv(&time, 5, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      GenPrms.set_start(time);

      GenPrms.set_usEarliest(get_us_time(time));
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_STOP_TIME:
    {
      log_writer.MPI(2,"MPI_TAG_SET_STOP_TIME");
      int time[5];
      MPI_Recv(&time, 5, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      GenPrms.set_stop(time);
      
      GenPrms.set_usLatest(get_us_time(time));
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_START_CORRELATE_NODE:
    {
      log_writer.MPI(2,"MPI_TAG_START_CORRELATE_NODE");
      int cmd;
      MPI_Recv(&cmd, 1, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
               
      start();           
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CONTROL_PARAM:
    {
      log_writer.MPI(2,"MPI_TAG_CONTROL_PARAM");
      MPI_Transfer mpi_transfer;
      mpi_transfer.receive_general_parameters(status,RunPrms,GenPrms,StaPrms);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_DELAY_TABLE:
    {
      log_writer.MPI(2,"MPI_TAG_DELAY_TABLE");
      MPI_Transfer mpi_transfer;
      DelayTable table;
      mpi_transfer.receive_delay_table(status, table);
      correlation_add_delay_table(table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

void 
Correlator_controller::start() {
  pthread_create(&correlator_thread, NULL, start_correlating, static_cast<void*>(this));
  running = true;
}

void *
Correlator_controller::start_correlating(void *self_) {
  Self *self = static_cast<Self *>(self_);
  self->correlate();

  int i=0; 
  MPI_Send(&i, 1, MPI_INT, 0,
           MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);

  return NULL;
}

void
Correlator_controller::correlate() {
  //Find Offsets
  log_writer << "Correlator_controller: Find offsets ..." << std::endl;
  assert((int)data_readers.size() == GenPrms.get_nstations());
  if (FindOffsets(data_readers, 1, 0) !=0) {
    std::cerr << "ERROR: FindOffsets, program aborted.\n";
    return;
  }

  log_writer << "Correlator_controller: Correlator bufs ..." << std::endl;
  if ( RunPrms.get_runoption() == 1 ) {
    //Process data for rank (=process identifier)
    CorrelateBufs(0, data_readers);
  }
  log_writer << "Correlator_controller: correlation done ..." << std::endl;
}
