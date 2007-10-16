/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Checks reading data from file using a Data_reader.
*/


#include <Data_reader_file.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

char *infile = "file://data/input.txt";
char *outfile = "file://output.txt";
#define BUFFSIZE 1000

#include <Semaphore_buffer.h>
#include <Data_writer_file.h>
#include <Data_reader2buffer.h>
#include <Buffer2data_writer.h>

// NGHK: remove
#include <Log_writer_cout.h>

int main(int argc, char *argv[]) {
  {
    {
      int bytes_read, bytes_written;
      char buff[BUFFSIZE];
  
      Data_reader_file reader(infile);
      Data_writer_file writer(outfile);
  
      bytes_read = reader.get_bytes(100, buff);
      assert(bytes_read==100);
      bytes_written = writer.put_bytes(bytes_read, buff);
      assert(bytes_read == bytes_written);
  
      bytes_read = reader.get_bytes(250, buff);
      assert(bytes_read==250);
      bytes_written = writer.put_bytes(bytes_read, buff);
      assert(bytes_read == bytes_written);
  
      for (int i=0; i<10; i++) {
        bytes_read = reader.get_bytes(50, buff);
        assert(bytes_read==50);
        bytes_written = writer.put_bytes(bytes_read, buff);
        assert(bytes_read == bytes_written);
      }
  
      bytes_read = reader.get_bytes(BUFFSIZE, buff);
      assert(bytes_read==BUFFSIZE);
      bytes_written = writer.put_bytes(bytes_read, buff);
      assert(bytes_read == bytes_written);
  
      while (!reader.eof()) {
        bytes_read = reader.get_bytes(BUFFSIZE, buff);
        bytes_written = 0;
        while (bytes_written != bytes_read) {
          bytes_written += writer.put_bytes(bytes_read, buff);
        }
        assert(bytes_read == bytes_written);
      }
    }
  
    //check output:
    std::string command = "diff ";
    command += infile+7; command += " "; command += outfile+7;
    int result = system(command.c_str());
  
    if (result != 0) {
      std::cout << "ERROR: Difference in files" << std::endl;
      exit(1);
    }
    
    remove(outfile);
  }
  
  { // Read from data_reader and write to data_writer using a buffer and two threads
    {
      typedef Buffer_element<char, 100>  value_type;
      boost::shared_ptr<Data_reader_file> 
        reader(new Data_reader_file(infile));
      boost::shared_ptr< Semaphore_buffer<value_type> > 
        buffer(new Semaphore_buffer<value_type>(1000));
      boost::shared_ptr<Data_writer_file> 
        writer(new Data_writer_file(outfile));
      
      Log_writer_cout log_writer;
      
      Data_reader2buffer<value_type>     reader2buffer;
      Buffer2data_writer<value_type>     buffer2writer;
      
      reader2buffer.set_data_reader(reader);
      reader2buffer.set_buffer(buffer);
      reader2buffer.start();
  
      buffer2writer.set_data_writer(writer);
      buffer2writer.set_buffer(buffer);
      buffer2writer.start();
      
      while (!reader2buffer.get_data_reader()->eof()) {
        usleep(100000);
      }
      while (!buffer2writer.get_buffer()->empty()) {
        usleep(100000);
      }
    }
  
    //check output:
    std::string command = "diff ";
    command += infile+7; command += " "; command += outfile+7;
    int result = system(command.c_str());
  
    if (result != 0) {
      std::cout << "ERROR: Difference in files" << std::endl;
      exit(1);
    }
    
    //remove(outfile);
  }
  return 0;
}
