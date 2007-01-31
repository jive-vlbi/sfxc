#include <Output_controller.h>
#include <iostream>
#include <assert.h>

#include <Data_writer_file.h>
#include <Data_reader_tcp.h>
#include <TCP_Connection.h>
#include <Queue_buffer.h>

Output_controller::Output_controller(Buffer<value_type> &buffer,
                                     Log_writer &log_writer)
  : Controller(log_writer), output_buffer(buffer), output_writer(NULL) {
}

Controller::Process_event_status
Output_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_OUTPUT_NODE_FILE:
    {
      log_writer.MPI(0, "MPI_TAG_SET_OUTPUT_NODE_FILE");
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      if (output_writer != NULL) delete output_writer;
      output_writer = new Data_writer_file(filename);

      if (output_writer != NULL) {
        start_output();
      }
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CREATE_OUTPUT_STREAM_TCP:
    {
      log_writer.MPI(0, "MPI_TAG_CREATE_OUTPUT_STREAM_TCP");
      
      int corr_rank;
      MPI_Recv(&corr_rank, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      Data_reader_tcp *data_reader = new Data_reader_tcp(1233);
      Input_stream input_stream;
      input_stream.reader      = data_reader;
      input_stream.buffer      = new Queue_buffer<value_type>();
      input_stream.slice_ready = false;

//      if (data_readers.size() <= (unsigned int)corr_rank) {
//        data_readers.resize(corr_rank+1);
//      }
//      if (data_readers[corr_rank] != NULL) {
//        delete(data_readers[corr_rank]);
//      }
      
      data_readers[corr_rank] = input_stream;
 
      TCP_Connection tcp_connection;
      std::vector<UINT64>  ip_addresses;
      tcp_connection.get_ip_addresses(ip_addresses);

      start_input(&data_readers[corr_rank]);

      ip_addresses.push_back(data_reader->get_port());
      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UNSIGNED_LONG, 
               corr_rank, MPI_TAG_SET_OUTPUT_STREAM_TCP, MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_WEIGHT_OUTPUT_STREAM:
    {
      log_writer.MPI(0, "MPI_TAG_SET_WEIGHT_OUTPUT_STREAM");
      int weight;
      MPI_Recv(&weight, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      input_streams_order[weight] = status.MPI_SOURCE;
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED:
    {
      log_writer.MPI(0, "MPI_TAG_OUTPUT_STREAM_ENDED");
      int weight;
      MPI_Recv(&weight, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      // NGHK: not thread save:      
      data_readers[status.MPI_SOURCE].buffer->produce();
      data_readers[status.MPI_SOURCE].buffer->produced(0);
      data_readers[status.MPI_SOURCE].slice_ready = true;
      
      write_buffers();     
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

void
Output_controller::write_buffers() {
  while ((!input_streams_order.empty()) &&
         (data_readers[input_streams_order.begin()->second].slice_ready)) {
    // output stream
    log_writer.MPI(0, "printing output");
    
    Input_stream &stream = data_readers[input_streams_order.begin()->second];
    while (!stream.buffer->empty()) {
      log_writer(0) << " ** printing output" << std::endl;
      int size;
      value_type& t_in = stream.buffer->consume(size);
      value_type& t_out = output_buffer.produce();
      assert(size <= t_in.size());
      memcpy(t_out.buffer(), t_in.buffer(), sizeof(value_type::value_type)*size);
      output_buffer.produced(size);
      stream.buffer->consumed();
    }
    
    input_streams_order.erase(input_streams_order.begin());
  }
}

void
Output_controller::start_output() {
  pthread_create(&output_thread, NULL, 
                 start_writing, static_cast<void*>(this));
}

void
Output_controller::start_input(Input_stream *reader) {
  pthread_t input_thread;
  pthread_create(&input_thread, NULL, 
                 start_reading, static_cast<void*>(reader));
}

void
Output_controller::stop_output() {
  output_buffer.produce();
  output_buffer.produced(0);
}
void *
Output_controller::start_writing(void *self_) {
  Self *self = static_cast<Self *>(self_);
  self->write_output();

  // Writing ended, notify controller node
  int i=0; 
  MPI_Send(&i, 1, MPI_INT32, 0,
           MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);

  return NULL;
}

void
Output_controller::write_output() {
  bool running = true;
  while (running) {
    int size;
    value_type &t = output_buffer.consume(size);
    output_writer->put_bytes(size, t.buffer());
    output_buffer.consumed();
    running = (size != 0);
  }
}
void *
Output_controller::start_reading(void *input_stream_) {
  Input_stream *input_stream = static_cast<Input_stream *>(input_stream_);
  int nRead = 1;
  while (nRead > 0) {
    value_type& elem = input_stream->buffer->produce();
    nRead = input_stream->reader->get_bytes(131072, elem.buffer());
    input_stream->buffer->produced(nRead);
  }
  return NULL;
}

