#include <Correlator_node.h>
#include <signal.h>
#include <iostream>
#include <InData.h>

#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

#include <Data_writer_file.h>
#include <Data_writer_tcp.h>

#include <utils.h>

#include <assert.h>

#include <MPI_Transfer.h>

Correlator_node_controller::Correlator_node_controller(Correlator_node &node)
 : Controller(node), 
   node(node)
{
   GenPrms.set_usStart(0);
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
  case MPI_TAG_CORRELATE_TIME_SLICE:
    {
      get_log_writer().MPI(2,"MPI_TAG_SET_TIME_SLICE");
      INT64 time[3];
      MPI_Recv(&time, 3, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      get_log_writer()(0) << "slicenr: " << time[0]
                          << ",  start: " << time[1] 
                          << ", duration: " << time[2] << std::endl;

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      GenPrms.set_usStart(time[1]);
      GenPrms.set_duration(time[2]);
      
      INT64 priority[] = {node.get_rank(), time[0]};
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
      int sn;
      mpi_transfer.receive_delay_table(status, table, sn);
      node.add_delay_table(sn, table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
