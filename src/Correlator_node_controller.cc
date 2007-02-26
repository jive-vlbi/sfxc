#include <Correlator_node.h>
#include <signal.h>
#include <iostream>
#include <ProcessData.h>
#include <InData.h>

#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

#include <Data_writer_file.h>
#include <Data_writer_tcp.h>

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

#include <MPI_Transfer.h>

Correlator_node_controller::Correlator_node_controller(Correlator_node &node)
 : Controller(node), 
   node(node)
//   output_buffer(output_buffer), curr_station(0),
//   data_writer(NULL)
{
   GenPrms.set_usStart(0);
   GenPrms.set_usStop(0);
   GenPrms.set_usEarliest(0);
}

Correlator_node_controller::~Correlator_node_controller()
{
}

Controller::Process_event_status 
Correlator_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_CONTROL_FILE:
    {
      get_log_writer().MPI(2,"MPI_TAG_SET_CONTROL_FILE: DEPRECATED");
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      if (initialise_control(filename, get_log_writer()) != 0) {
        get_log_writer().MPI(2,"Initialisation using control file failed");
      }

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_TIME_SLICE:
    {
      get_log_writer().MPI(2,"MPI_TAG_SET_TIME_SLICE");
      INT64 time[2];
      MPI_Recv(&time, 2, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      get_log_writer()(0) << "  start: " << time[0] 
                          << ", stop : " << time[1] << std::endl;

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      GenPrms.set_usStart(time[0]);
      GenPrms.set_usEarliest(time[0]);
      
      GenPrms.set_usStop(time[1]);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
    
  case MPI_TAG_START_CORRELATE_NODE:
    {
      get_log_writer().MPI(2,"MPI_TAG_START_CORRELATE_NODE");
      int slice;
      MPI_Recv(&slice, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      INT64 priority[] = {node.get_rank(), slice};
      MPI_Send(&priority, 2, MPI_INT64, 
               RANK_OUTPUT_NODE, MPI_TAG_OUTPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);
      
      node.start_correlating();           

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CONTROL_PARAM:
    {
      get_log_writer().MPI(2,"MPI_TAG_CONTROL_PARAM");
      MPI_Transfer mpi_transfer;
      mpi_transfer.receive_general_parameters(status,RunPrms,GenPrms,StaPrms);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_DELAY_TABLE:
    {
      get_log_writer().MPI(2,"MPI_TAG_DELAY_TABLE");
      MPI_Transfer mpi_transfer;
      DelayTable table;
      mpi_transfer.receive_delay_table(status, table);
      correlation_add_delay_table(table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
//  case MPI_TAG_SET_OUTPUT_STREAM_TCP:
//    {
//      log_writer.MPI(2,"MPI_TAG_SET_OUTPUT_STREAM_TCP");
//      int size;
//      MPI_Get_elements(&status, MPI_UNSIGNED_LONG, &size);
//      assert(size > 1);
//      UINT64 ip_addr[size];
//      MPI_Recv(&ip_addr, size, MPI_UNSIGNED_LONG, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//      
//      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
//      assert(status.MPI_TAG == status2.MPI_TAG);
//
//      // last element is the port number:
//      Data_writer *writer = new Data_writer_tcp(ip_addr, size-1, ip_addr[size-1]);
//      assert(writer != NULL);
//      node.set_data_writer(writer);      
//      
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
//  case MPI_TAG_SET_OUTPUT_NODE_FILE:
//    {
//      log_writer.MPI(2,"MPI_TAG_SET_OUTPUT_NODE_FILE");
//      int size;
//      MPI_Get_elements(&status, MPI_CHAR, &size);
//      assert(size > 0);
//      char filename[size];
//      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//  
//      Data_writer *writer = new Data_writer_file(filename);
//      set_data_writer(*writer);
//      //GenPrms.set_corfile(filename);
//  
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
