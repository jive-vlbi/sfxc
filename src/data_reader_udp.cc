/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *  This file contains:
 *     - the definition of the Data_reader_udp object.
 */
#include <poll.h>
#include "data_reader_udp.h"
#include "memory_pool.h"

void Data_reader_udp::Reading_thread::add_packet(int i, pool_element p)
{
    //std::cout << "Adding: " << i << " value:" << p.data().get_counter() << std::endl;

    //value +=base_;
    if ( i < packet_base_ )
    {
        packet_dropped_++;
        //std::cout << "Packet dropped: " << i << std::endl;
        return;
    }

		static int largest=0;
		if(i>largest){
				largest=i;
		}else{
				std::cout << "incoming packet: " << i << std::endl;
				packet_outoforder_++;
		}

    //std::cout << "\n";
    unsigned int ii = i-packet_base_;

    if ( ii < size_ )
    {
        //std::cout << "setting value: " << ii << "("<< ii%size_ << ") value:" << value << std::endl;
        //printf("Received: %p\n", p);
        if ( ringbuffer_[i] != NULL )
        {
            packet_duplicated_++;
            //std::cout << "Duplicate ?: " << ringbuffer_[i]->get_counter() << std::endl;
        }
        else
        {
				    ringbuffer_[i]= p;
        }
    }
    else
    {
        int zero=0;
        unsigned int nb = i - size_;
        //value -=base_;
        //if ( packet_base_ % 1000 == 0)
        //std::cout << "SUCETTE:" << packet_base_ << ":" <<nb << " lost packet: " << packet_lost_  << " vs " << packet_received_  << " so: " << packet_received_*100.0/( packet_received_+packet_lost_) << std::endl;

        for (packet_base_;packet_base_<=nb;packet_base_++)
        {
            if ( ringbuffer_[packet_base_] != NULL )
            {
                //std::cout << "packet emited: " << base_ << std::endl;
                //std::cout << "packet emited: " << packet_base_ << " real cpt:" << ringbuffer_[packet_base_]->get_counter()  << "omg: " << nb << std::endl;
                parent_queue_.push( ringbuffer_[packet_base_] );

                ringbuffer_[packet_base_] = NULL;
                packet_received_++;
            }
            else
            {
                parent_queue_.push( NULL );
                packet_lost_++;
                //std::cout << "packet emited: " << packet_base_  << "(BUT LOST)" << std::endl;
            }
            //packet_base_++;
        }
        //value += base_;
				//std::cout << "incoming packet_b: " << i << std::endl;
        ringbuffer_[i] = p;

    }
}



Data_reader_udp::Data_reader_udp(int socket) :
        psize_(1400), size_(1000),
        pool_(size_+1, NO_RESIZE, Packet_fixed_size::PAllocator::create(psize_) ),
        reading_thread_(pool_, queue_, psize_, size_, socket)
{
    socket_=socket;
    isfirst = true;
    reading_thread_.start();
    old_current_idx_=0;
}

Data_reader_udp::~Data_reader_udp()
{
	wait( reading_thread_ );
}

bool Data_reader_udp::eof()
{
    return false;
}



bool Data_reader_udp::can_read()
{
    //     int fd;           /* file descriptor */
    //     short events;     /* requested events */
    //     short revents;    /* returned events */
    //   };

    pollfd fds[1];
    fds[0].fd = socket_;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, /*timeout in miliseconds*/ 0);
    if (ret > 0)
    {
        return ((fds[0].revents & POLLIN) != 0);
    }

    return false;
}

void Data_reader_udp::Reading_thread::do_execute()
{
		int sbuf = 1024*1024;
		int  dbuf = 0;
		socklen_t olen = sizeof(sbuf);
		//CHECK_ZERO( setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &sbuf, olen) );
		//CHECK_ZERO( getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &dbuf, &olen) );
		std::cout << "Sending buffer will be: " <<  (1.0*dbuf)/(1024*1024) << "MB" <<std::endl;

		pool_element el=parent_pool_.allocate();
		recvmsg(socket_, &(el->get_message()), 0 );

		packet_base_ = el->get_counter();
		prev_packet_ = el->get_counter();

		std::cout << "PACKET_BASE: " << packet_base_ << std::endl;
		add_packet( el->get_counter(), el );

		while(isrunning_)
		{
			//if( parent_pool_.empty()){
			//		std::cout << "The memory_pool is not big enough !" << std::endl;
			//}
			pool_element el=parent_pool_.allocate();
			recvmsg(socket_, &(el->get_message()), 0 );
			add_packet( el->get_counter(), el );
		}
}

int Data_reader_udp::do_get_bytes(size_t readb, char *buffer)
{
		static int old_p;
		static int prev_pack=-1;

    if ( reading_thread_.packet_base_-old_p > 1000)
    {
				old_p = reading_thread_.packet_base_;
        std::cout << "Reading info:  " << std::endl;
        std::cout << " packet received:"<< reading_thread_.packet_received_ ;
        std::cout << " packet lost: "<< reading_thread_.packet_lost_ << std::endl;
				std::cout << " packet ooo:"<< reading_thread_.packet_outoforder_ ;
        std::cout << " packet duplicated: "<< reading_thread_.packet_duplicated_;
        double total = (reading_thread_.packet_received_+reading_thread_.packet_lost_);
        double percentage= (100.0*reading_thread_.packet_lost_)/total;
        std::cout << " ( " << percentage <<  "%)" << std::endl;
    }


    if (buffer == NULL)
    {
        size_t buff_size = 1000000;
        readb = (readb < buff_size ? readb : buff_size);
        char buff[(int)readb];
        //int ret = datareader_->get_bytes(readb, buff);
        return readb;
    }

		//std::cout << " COINCOIN! "<< queue_.size() << std::endl;
		if( queue_.empty() ) return 0;

    uint64_t current_idx=0;
		//std::cout << " Let's go ! " << " total size to read: " <<  readb << " previous p: " << old_current_idx_  << std::endl;

    // We first have to check if there is some datas that remains from
    // the previous call to this function.
    if ( old_current_idx_ != 0 )
    {
        //std::cout << " SECTION 1! "<< queue_.size() << " old_current_idx " << old_current_idx_ << std::endl;

        // If this is the case we copy the "saved amount of byte
        // to the new given buffer.
        uint64_t s=MIN( (old_current_idx_), readb );
				//std::cout << " SECTION 1! "<< queue_.size() << " soo read: " << s << std::endl;

        // retreive from the waiting queue.
        pool_element element=queue_.front();

        if ( element != NULL )
        {
						assert( element->get_counter() == reading_thread_.prev_packet_ );

            // we can copy to the buffer the available byte in this
            // packet.
            memcpy( buffer, element->buffer(), s );

            // increase the amount of byte_read;
            current_idx += s;

            // decrease the amount of remaining bytes
            old_current_idx_ -= s;

            // if all the byte in this packed have been processed
            // we can remove this paquet from the queue.
            if ( old_current_idx_==0 )
            {
                queue_.pop();
                pool_.release( element );
								reading_thread_.prev_packet_++;
            }
        }
        else
        {
            // increase the amount of byte_read;
            current_idx += s;

            // decrease the amount of remaining bytes
            old_current_idx_ -= s;

						memset( buffer, 0, s);

            // if all the byte in this packed have been processed
            // we can remove this paquet from the queue.
            if ( old_current_idx_ )
            {
                queue_.pop();
 								reading_thread_.prev_packet_++;
            }
        }

    }


    // we loop as long as we have not received a complete packet
    // and as long as we have something to read.
		//std::cout << " BON ET ALORS:"  << readb << "-current_idx["<< current_idx << "] psize[" << psize_ << "]" << std::endl;

    int iter = (readb-current_idx)/psize_;
    iter = MIN(iter, queue_.size());

    for (int i=0;i<iter;i++)
    {
        //std::cout << " SECTION 2! "<< queue_.size() << " iter:" << iter << std::endl;

        // retreive from the waiting queue.
        pool_element element=queue_.front();

        if ( element != NULL )
        {
						assert(reading_thread_.prev_packet_ == element->get_counter() );

            // we copy the complete packet to the destination buffer
            // this copy is a pity but I have no idea how to remove it
            // without screwing the design of the data_reader.
            memcpy( &(buffer[current_idx]), element->buffer(), psize_ );

            pool_.release( element );
        }
        else
        {
            //std::cout << "SHOULD randomize the data" << std::endl;
						memset( &(buffer[current_idx]), 0, psize_);
        }

        // advance the pointer by the size of the packet.
        current_idx += psize_;
        queue_.pop();
				reading_thread_.prev_packet_++;
    }

    //std::cout << " ENFIN: "<< queue_.size() << " remaining: "<< current_idx << " vs " << readb << std::endl;

    // some bytes remains in the packet that cannot be stored
    // in the user provided buffer. These are saved
    if ( current_idx < psize_ )
    {
				//std::cout << " mais c'est des chaussures: " << readb << std::endl;

        old_current_idx_ = psize_-current_idx;
    }
    else
    {
        // reseting the "memory" to indcate half-processed
        // packet.
        old_current_idx_ = 0;
    }

    return current_idx;
}
