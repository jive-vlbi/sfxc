// Output_node_controller is defined in Output_node.h:
#include <Output_node.h>

#include <iostream>
#include <assert.h>

#include <Data_writer_file.h>
#include <Data_reader_tcp.h>
#include <TCP_Connection.h>
#include <Queue_buffer.h>

Output_node_controller::Output_node_controller(Output_node &node)
  : Controller(node.get_log_writer()), node(node) {
    
}

Controller::Process_event_status
Output_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_OUTPUT_STREAM_SET_PRIORITY:
    {
      log_writer.MPI(0, print_MPI_TAG(status.MPI_TAG));
      INT64 weight[2];
      MPI_Recv(&weight, 2, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      // Create an output buffer:
      node.create_buffer(weight[0]);
      node.set_weight_of_input_stream(weight[0], weight[1]);
      node.set_status();
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED:
    {
      log_writer.MPI(0, print_MPI_TAG(status.MPI_TAG));
      int rank;
      MPI_Recv(&rank, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      node.time_slice_finished(rank);      
      node.set_status();
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

//void
//Output_node_controller::write_buffers() {
//  while ((!input_streams_order.empty()) &&
//         (data_readers[input_streams_order.begin()->second].slice_ready)) {
//    // output stream
//    log_writer.MPI(0, "printing output");
//    
//    Input_stream &stream = data_readers[input_streams_order.begin()->second];
//    while (!stream.buffer->empty()) {
//      log_writer(0) << " ** printing output" << std::endl;
//      int size;
//      value_type& t_in = stream.buffer->consume(size);
//      value_type& t_out = output_buffer.produce();
//      assert(size <= t_in.size());
//      memcpy(t_out.buffer(), t_in.buffer(), sizeof(value_type::value_type)*size);
//      output_buffer.produced(size);
//      stream.buffer->consumed();
//    }
//    
//    input_streams_order.erase(input_streams_order.begin());
//  }
//}
//
//void
//Output_node_controller::start_output() {
//  pthread_create(&output_thread, NULL, 
//                 start_writing, static_cast<void*>(this));
//}
//
//void
//Output_node_controller::start_input(Input_stream *reader) {
//  pthread_t input_thread;
//  pthread_create(&input_thread, NULL, 
//                 start_reading, static_cast<void*>(reader));
//}
//
//void
//Output_node_controller::stop_output() {
//  output_buffer.produce();
//  output_buffer.produced(0);
//}
//void *
//Output_node_controller::start_writing(void *self_) {
//  Self *self = static_cast<Self *>(self_);
//  self->write_output();
//
//  // Writing ended, notify controller node
//  int i=0; 
//  MPI_Send(&i, 1, MPI_INT32, 0,
//           MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
//
//  return NULL;
//}
//
//void
//Output_node_controller::write_output() {
//  bool running = true;
//  while (running) {
//    int size;
//    value_type &t = output_buffer.consume(size);
//    output_writer->put_bytes(size, t.buffer());
//    output_buffer.consumed();
//    running = (size != 0);
//  }
//}
//void *
//Output_node_controller::start_reading(void *input_stream_) {
//  Input_stream *input_stream = static_cast<Input_stream *>(input_stream_);
//  int nRead = 1;
//  while (nRead > 0) {
//    value_type& elem = input_stream->buffer->produce();
//    nRead = input_stream->reader->get_bytes(131072, elem.buffer());
//    input_stream->buffer->produced(nRead);
//  }
//  return NULL;
//}
//
//void 
//Output_node_controller::set_data_reader(int num, Input_stream &stream) {
//  if (data_readers.size() <= (unsigned int)num) data_readers.resize(num+1);
//  data_readers[num] = stream;
//}
