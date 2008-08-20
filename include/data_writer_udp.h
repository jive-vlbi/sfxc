/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *  This file contains:
 *     - Implementation of a data_writer based on UDP/IP protocol. This is not
 *       a reliable transport protocol beware. The missing informations are
 *       replace 'dummy' informations.
 */
#ifndef DATA_WRITER_UDP_H_INCLUDED
#define DATA_WRITER_UDP_H_INCLUDED

#include "memory_pool.h"
#include "memory_pool_elements.h"
#include "threadsafe_queue.h"
#include "udp_packet.h"


class Data_writer_udp : public Data_writer {
  Packet_counter packetF_;
  Packet_counter packetH_;

  int psize_;
  int socket_;
  int old_size_;
  unsigned char* old_buffer_;
  uint64_t counter_;
  int bufferA_;
  int bufferB_;
  int bufferC_;

  // interpacket delay in microseconds
  int delay_;
public:
  Data_writer_udp( int socket ) :
      socket_(socket) {
    psize_ = 1400;
    counter_ = 10; //random();
    old_size_ = 0;
    old_buffer_ = new unsigned char[ psize_*2 ];
    // the buffer is used to send complete packet (full size).
    bufferA_ = packetF_.add_sub_buffer( old_buffer_, psize_ );

    // this packet is used to send packets composed of two chunks !
    // if you want more this means you are a realy sadistic !
    bufferB_ = packetH_.add_sub_buffer( old_buffer_, psize_ );
    bufferC_ = packetH_.add_sub_buffer( old_buffer_, 0 );

    delay_ = 4;

    configure(0,0);
  }

  ~Data_writer_udp() {
    delete[] old_buffer_;
  }

  void configure(int priority, int buffersize) {
    int sbuf = 1024*1024*4;
    int  dbuf = 0;
    socklen_t olen = sizeof(sbuf);
    //CHECK_ZERO( setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &sbuf, olen) );
    //CHECK_ZERO( getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &dbuf, &olen) );
    std::cout << "Sending buffer will be: " <<  (1.0*dbuf)/(1024*1024) << "MB" <<std::endl;
  }

  void set_interpacket_delay(unsigned int delay) {
    delay_ = delay;
  }

  unsigned int get_interpacket_delay() {
    return delay_;
  }


  /***************************************************************************
  *
  * The function will throw and exception in case of problem ! :)
  *
  ***************************************************************************/
  size_t do_put_bytes(size_t bytestoput, const char* buffer) {
    assert( buffer != NULL );
    assert( old_buffer_ != NULL );
    assert( bytestoput >= psize_ && "The other case should be implemented soon !" );
    size_t bytes=bytestoput;
    size_t curr=0;

    // check if it remains some bytes from the previous do_put_bytes
    // call.
    if ( old_size_ != 0) {
      // we send the packet
      packetH_.set_counter( counter_++ );
      packetH_.set_buffer( bufferB_, old_buffer_, old_size_ );
      packetH_.set_buffer( bufferC_, (unsigned char*)buffer, psize_-old_size_ );

      if ( sendmsg(socket_, &(packetH_.get_message()),  0 ) < 0 ) {
        std::cout << "I don't have received the packet: "  << socket_ << std::endl;
        perror("Helo::1 ");
        MTHROW("Problem !");
      }

      usleep(delay_);

      bytes -= (psize_-old_size_);
      curr += (psize_-old_size_);
    }

    // we then send the other buffer elements.
    while ( bytes > psize_ ) {
      packetF_.set_counter( counter_++ );
      packetF_.set_buffer( bufferA_, (unsigned char*)&(buffer[curr]), psize_ );

      if ( sendmsg(socket_, &(packetF_.get_message()),  0 ) < 0 ) {
        std::cout << "I don't have received the packet: "  << socket_  << std::endl;
        perror("Helo::2 ");
        MTHROW("Problem !");
      }

      curr += psize_;
      bytes -= psize_;
      usleep(delay_);
    }

    // if there is not enough bytes to fill the last packet
    // these data are saved for the next call.
    if ( bytes != 0 ) {
      old_size_ = bytes;
      memcpy(old_buffer_, (unsigned char*)&buffer[curr], bytes);
    }

    return bytestoput;
  }

  bool can_write() {
    return true;
  }
};

#endif // DATA_WRITER_UDP_H_INCLUDED
