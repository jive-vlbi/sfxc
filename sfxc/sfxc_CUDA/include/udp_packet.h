/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 */

#ifndef UDP_PACKET_H__
#define UDP_PACKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "align_malloc.h"
#include "connexion.h"
#include "default_allocator.h"

class Udp_packet {
#define MAX_BUFFER 5
  struct msghdr envelope_;

  // 5 is here just to insure we have enough space
  // to store mutliple destination buffer.
  struct iovec iovect_[MAX_BUFFER];
  unsigned int bufferid_;
  unsigned int totalsize_;
public:
  Udp_packet() {
    envelope_.msg_name       = 0;
    envelope_.msg_namelen    = 0;
    envelope_.msg_iov        = &iovect_[0];
    envelope_.msg_iovlen     = 0;
    envelope_.msg_control    = 0;
    envelope_.msg_controllen = 0;
    envelope_.msg_flags      = 0;
    bufferid_ = 0;
    totalsize_ = 0;
  }

  int add_sub_buffer(unsigned char* data, unsigned int size) {
    assert( bufferid_ < MAX_BUFFER );
    //printf("adding : %d size:%d\n", bufferid_, size );

    iovect_[ bufferid_ ].iov_base = data;
    iovect_[ bufferid_ ].iov_len = size;
    envelope_.msg_iovlen++;

    bufferid_++;
    totalsize_ += size;
    return bufferid_-1;
  }


  void set_buffer(unsigned int i, unsigned char* data, unsigned int size) {
    assert( i < bufferid_ );

    totalsize_ -= iovect_[ i ].iov_len;
    iovect_[ i ].iov_base = data;
    iovect_[ i ].iov_len = size;
    totalsize_ += iovect_[ i ].iov_len;
  }

  struct msghdr& get_message() {
    return envelope_;
  }

  friend class Packet_writer;
  friend class Packet_reader;
};

class Packet_counter : public Udp_packet {
  unsigned int counter_;
  bool valid_;
public:
  Packet_counter() {
    set_counter(0);
    add_sub_buffer((unsigned char*)&counter_, sizeof(counter_) );
    valid_=true;
  }

  void set_validity(bool valid) {
    valid_=valid;
  }

  bool is_valid() {
    return valid_;
  }

  void set_counter(unsigned int value) {
    counter_ = htonl(value);
  }

  unsigned int get_counter() {
    return ntohl(counter_);
  }
};

class Packet_fixed_size;
class Packet_fixed_size : public Udp_packet {
  unsigned int counter_;
  bool valid_;
  unsigned char* buffer_;
public:
  Packet_fixed_size(int size) {
    set_counter(0);
    add_sub_buffer((unsigned char*)&counter_, sizeof(counter_) );

    buffer_ = (unsigned char*)aligned_malloc(size);
    CHECK_MALLOC( buffer_ );

    add_sub_buffer(buffer_ , size);
    valid_=true;
  }

  ~Packet_fixed_size() {
    aligned_free(buffer_);
  }

  unsigned char* buffer() {
    return buffer_;
  }

  void set_validity(bool valid) {
    valid_=valid;
  }
  bool is_valid() {
    return valid_;
  }

  void set_counter(unsigned int value) {
    counter_ = htonl(value);
  }

  unsigned int get_counter() {
    return ntohl(counter_);
  }

class PAllocator : public Allocator<Packet_fixed_size> {
  public:
    typedef boost::shared_ptr< PAllocator > SelfPtr;
    typedef Packet_fixed_size   Type;
    typedef Packet_fixed_size* pType;

    PAllocator(size_t default_size) {
      default_size_ = default_size;
    }
    ~PAllocator() {}

    pType allocate() {
      return new Packet_fixed_size(default_size_);
    }

    static SelfPtr create(size_t size) {
      return SelfPtr( new PAllocator(size) );
    }

  private:
    size_t default_size_;
    PAllocator();
  };


};


class Packet_writer {
  int socket_;
public:
  Packet_writer(int socket) : socket_(socket) {}

  void send_message(Udp_packet& p) {
    if ( sendmsg(socket_, &(p.get_message()),  0 ) < 0 ) {
      std::cout << "I don't have received the packet: " << std::endl;
      perror("Helo:: ");
    }
  }
};


class Packet_reader {
  EndpointIP* ep_;
public:
  Packet_reader(EndpointIP* ep) {
    ep_ = ep;
  }

  void receive_message(Udp_packet& p) {
    //char buffer[2000];
    //memset(buffer, 2000, 0);
    //std::cout << " Bison: " << p.bufferid_ << " totalsize:" << p.totalsize_ << std::endl;
    //std:: cout << recv(ep_->socket(), buffer, 1028, NULL) << std::endl;
    if ( recvmsg( ep_->socket(), &(p.get_message()),  0 ) < 0 ) {
      std::cout << "I don't have received the packet: " << std::endl;
      perror("Helo:: ");
    }
  }

};

#endif // UDP_PACKET_H__
