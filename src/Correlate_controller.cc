#include <Correlate_controller.h>
#include <signal.h>
#include <iostream>
#include <ProcessData.h>
#include <InData.h>

#include <Data_reader_file.h>
#include <utils.h>

/// TODO: NGHK: REMOVE
#include <utils.h>

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

Correlate_controller::Correlate_controller(Buffer<output_value_type> &output_buffer)
 : output_buffer(output_buffer), running(false), curr_station(0)
{
}

Correlate_controller::~Correlate_controller()
{
}

int 
Correlate_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
//  sliceStartTime[0] = 0;
//  sliceStopTime[0] = 0;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_STATION_NUMBER:
    {
      std::cout << "MPI_TAG_SET_STATION_NUMBER" << std::endl;
      int station;
      MPI_Recv(&station, 1, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      curr_station = station;
      if (data_readers.capacity() <= static_cast<size_t>(curr_station)) {
        data_readers.reserve(curr_station+1);
      }
      return 0;
    }
  case MPI_TAG_SET_INPUT_NODE_FILE:
    {
      std::cout << "MPI_TAG_SET_INPUT_NODE_FILE" << std::endl;
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
      return 0;
    }
  case MPI_TAG_SET_OUTPUT_NODE_FILE:
      std::cout << "MPI_TAG_SET_OUTPUT_NODE_FILE" << std::endl;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      GenPrms.corfile = filename;

      return 0;
  {
  }
  case MPI_TAG_SET_CONTROL_FILE:
    {
      std::cout << "MPI_TAG_SET_CONTROL_FILE: DEPRECATED" << std::endl;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      std::cout << "FILENAME: " << filename << std::endl;

      if (initialise_control(filename) != 0) {
        std::cout << "Initialisation using control file failed" << std::endl;
      }

      return 0;
    }
  case MPI_TAG_SET_START_TIME:
    {
      std::cout << "MPI_TAG_SET_START_TIME" << std::endl;
      int time[5];
      MPI_Recv(&time, 5, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      GenPrms.yst = time[0];
      GenPrms.dst = time[1];
      GenPrms.hst = time[2];
      GenPrms.mst = time[3];
      GenPrms.sst = time[4];

      sliceStartTime[0] = get_us_time(time);
      return 0;
    }
  case MPI_TAG_SET_STOP_TIME:
    {
      std::cout << "MPI_TAG_SET_STOP_TIME" << std::endl;
      int time[5];
      MPI_Recv(&time, 5, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      GenPrms.ysp = time[0];
      GenPrms.dsp = time[1];
      GenPrms.hsp = time[2];
      GenPrms.msp = time[3];
      GenPrms.ssp = time[4];
      
      sliceStopTime[0] = get_us_time(time);
      return 0;
    }
  case MPI_TAG_START_CORRELATE_NODE:
    {
      std::cout << "MPI_TAG_START_CORRELATE_NODE" << std::endl;
      int cmd;
      MPI_Recv(&cmd, 1, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
               
      start();           
      return 0;
    }
  case MPI_TAG_CONTROL_PARAM:
    {
      std::cout << "MPI_TAG_CONTROL_PARAM" << std::endl;
      MPI_Transfer mpi_transfer;
      mpi_transfer.receive_general_parameters(status);
      return 0;            
    }
  default:
    {
      return 1;
    }
  }
}

void 
Correlate_controller::start() {
  pthread_create(&correlate_thread, NULL, start_correlating, static_cast<void*>(this));
  running = true;
}

void *
Correlate_controller::start_correlating(void *self_) {
  Self *self = static_cast<Self *>(self_);
  self->correlate();

  int i=0; 
  MPI_Send(&i, 1, MPI_INT, 0,
           MPI_MSG_CORRELATE_ENDED, MPI_COMM_WORLD);

  return NULL;
}

void
Correlate_controller::correlate() {
  //Find Offsets
  std::cout << "Correlate_controller: Find offsets ..." << std::endl;
  assert((int)data_readers.size() == GenPrms.get_nstations());
  if (FindOffsets(data_readers, 1, 0) !=0) {
    std::cerr << "ERROR: FindOffsets, program aborted.\n";
    return;
  }

  std::cout << "Correlate_controller: Correlate bufs ..." << std::endl;
  if ( RunPrms.get_runoption() == 1 ) {
    //Process data for rank (=process identifier)
    CorrelateBufs(0, data_readers);
  }
  std::cout << "Correlate_controller: correlation done ..." << std::endl;
}
