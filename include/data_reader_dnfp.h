#ifndef DATA_READER_DNFP_H
#define DATA_READER_DNFP_H

#include "connexion.h"

#include "exception_common.h"
#include "data_reader_socket.h"
#include "data_writer_socket.h"
#include "data_reader_blocking.h"

//typedef DataWriter_socket* pDataWriter_socket;
//typedef DataReader_socket* pDataReader_socket;

class Data_reader_dnfp : public Data_reader
{
public:
  Data_reader_dnfp(pInterfaceIP interface, const std::string url)
  {
    m_url = url;
    get_info(url, m_serverip, m_serverport, m_filename);

    std::cout << "INFO: " << m_serverip << " et " << m_serverport << " et " << m_filename;

    connect(interface);
    negociate();
    m_isEof = false;
  }

  Data_reader_dnfp(const std::string url)
  {
    m_url = url;
    get_info(url, m_serverip, m_serverport, m_filename);

    std::cout << "INFO: " << m_serverip << " et " << m_serverport << " et " << m_filename;

    connect(Network::get_first_interface());
    negociate();
    m_isEof = false;
  }


  bool eof(){ return m_isEof; }

void get_info(const std::string url, String& server, String& port, String& filename)
{
  if( url.find("dnfp://") < 0 ) MTHROW("INVALID URL for dnfp protocol");
  String tmp = url.substr(7);
  int serverb = tmp.find(":");
  if( serverb < 0 ) MTHROW("INVALID URL for dnfp protocol, missing port");
  server = tmp.substr(0, serverb);
  tmp = tmp.substr(serverb+1);
  serverb = tmp.find("/");
  if( serverb < 0 ) MTHROW("INVALID URL for dnfp protocol, missing port");
  port = tmp.substr(0, serverb);
  filename = tmp.substr(serverb);
}

  virtual void closef(){
      std::cout << "closing files" << std::endl;
      int32_t cmd=-1;
      *m_writer << cmd;
  }

  void connect(pInterfaceIP interface){
    if( interface == NULL ) MTHROW("No valid interface to host");
    pConnexion connexion= interface->connect_to( m_serverip, m_serverport );
    m_reader = new DataReader_socket( connexion->socket() );
    m_writer = new DataWriter_socket( connexion->socket() );
    m_breader  = new Data_reader_blocking(m_reader);
  }

  void negociate(){
    std::string response;
    *m_writer << String("PING");
    *m_breader >> response;
    std::cout << "The server respond me:" << response << std::endl;
    *m_writer << m_filename;
    *m_breader >> response;
    if( strcmp( response.c_str(), "OK") ) MTHROW("The server refuse to give me this file");
  }

  size_t do_get_bytes(size_t readb, char *buffer){
    int32_t readbyte = readb;
    *m_writer << readbyte;
    m_breader->get_bytes(readb, buffer);
    uint32_t eof;
    *m_breader >> eof;
    if( eof == 1 ) m_isEof = 1;
    return readb;
  }

  private:
    String m_filename;
    String m_serverip;
    String m_serverport;
    String m_url;

    bool m_isEof;

    DataWriter_socket* m_writer;
    DataReader_socket* m_reader;
    Data_reader_blocking* m_breader;
};


#endif // DATA_READER_DNFP_H
