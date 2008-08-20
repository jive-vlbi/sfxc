/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *  This file contains:
 *     - the definition of the Data_reader_dnfp object.
 */
#include "data_reader_dnfp.h"
#include "data_reader_udp.h"
#include "data_reader_blocking.h"

Data_reader_dnfp::Data_reader_dnfp(pInterfaceIP interface,
                                   const std::string url) {
  url_ = url;
  parse_url(url, serverip_, serverport_, filename_);

  std::cout << "INFO: " << serverip_ << " et " << serverport_ << " et " << filename_;

  connect(interface);
  negociate();
  isEof_ = false;
}

Data_reader_dnfp::Data_reader_dnfp(const std::string url) {
  url_ = url;
  parse_url(url, serverip_, serverport_, filename_);

  std::cout << "INFO: " << serverip_ << " et " << serverport_ << " et " << filename_;

  connect(Network::get_any_interface());
  negociate();
  isEof_ = false;
}


bool Data_reader_dnfp::eof() {
  return isEof_ || datareader_->eof() || ctrlreader_->eof() ;
}

void Data_reader_dnfp::parse_url(const std::string url,
                                 String& server, String& port,
                                 String& filename) {
  protocol_ = "tcp";

  if ( url.find("dnfp://") < 0 ) MTHROW("INVALID URL for dnfp protocol");
  String tmp = url.substr(7);
  int serverb = tmp.find(":");
  if ( serverb < 0 ) MTHROW("INVALID URL for dnfp protocol, missing port");
  server = tmp.substr(0, serverb);
  tmp = tmp.substr(serverb+1);
  serverb = tmp.find("/");
  if ( serverb < 0 ) MTHROW("INVALID URL for dnfp protocol, missing port");
  port = tmp.substr(0, serverb);

  tmp = tmp.substr(serverb+1);
  if ( tmp.find("?") != std::string::npos ) {
    serverb  = tmp.find("?");
    filename = tmp.substr(0,serverb);
    std::string options  = tmp.substr(serverb+1);
    std::cout << "OPTIONS: " << options << std::endl;
    if ( options.find("proto=") >= 0 ) {
      std::cout << "The protocol is finally:" << options.substr(6) << std::endl;
      protocol_ = options.substr(6);
      if ( protocol_ != "tcp" && protocol_ != "udp") {
        std::cout << "This is not a supported protocol:" << protocol_ << std::endl;
        MTHROW("Invalid protocol !");
      }
    }
  } else {
    filename = tmp;
  }
}


void Data_reader_dnfp::connect(pInterfaceIP interface) {
  if ( interface == NULL ) MTHROW("No valid interface to host");
  std::cout << "DNFP-Message: Connecting to:"+serverip_+":"+serverport_;
  std::cout << " from interface:"+interface->name() << std::endl;
  pConnexion connexion= interface->connect_to( serverip_, serverport_ );
  ctrlreader_ = new Data_reader_socket( connexion->socket() );
  ctrlwriter_ = new Data_writer_socket( connexion->socket() );
  ctrlbreader_  = new Data_reader_blocking( ctrlreader_ );

  // By default the data are going through the same
  // connexion as the control command. The default
  // are overridden when using the udp protocol.
  datareader_  = ctrlreader_;
  databreader_ = ctrlbreader_;
  datawriter_  = ctrlwriter_;
}

void Data_reader_dnfp::negociate() {
  std::string response;
  *ctrlwriter_ << String("data_reader_dnfp");
  *ctrlbreader_ >> response;
  std::cout << "Server responded:" << response << std::endl;

  // first we send the file to get
  *ctrlwriter_ << filename_;

  // the the protocol type
  *ctrlwriter_ << protocol_;

  // then its parameters.
  if ( protocol_ == "udp") {
    EndpointIP* ep=Network::create_endpoint();
    *ctrlwriter_ << String("localhost");
    *ctrlwriter_ << String( itoa( ep->get_port() ) );

    datareader_ = new Data_reader_udp( ep->get_socket() );
    databreader_ = new Data_reader_blocking( datareader_ );
  }

  // and we wait for the reply
  *ctrlbreader_ >> response;

  if ( strcmp( response.c_str(), "OK") ) MTHROW("The server refuse to serve the file");
  std::cout << "You are not throwing an exception ?"<< std::endl;
}

int Data_reader_dnfp::do_get_bytes(size_t readb, char *buffer) {
  if (buffer == NULL) {
    size_t buff_size = 1000000;
    readb = (readb < buff_size ? readb : buff_size);
    char buff[(int)readb];
    int ret = datareader_->get_bytes(readb, buff);
    return ret;
  }

  int ret = datareader_->get_bytes(readb, buffer);
  return ret;
}
