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
    parse_url(url, m_serverip, m_serverport, m_filename);

    std::cout << "INFO: " << m_serverip << " et " << m_serverport << " et " << m_filename;

    connect(interface);
    negociate();
    m_isEof = false;
  }

  Data_reader_dnfp(const std::string url)
  {
    m_url = url;
    parse_url(url, m_serverip, m_serverport, m_filename);

    std::cout << "INFO: " << m_serverip << " et " << m_serverport << " et " << m_filename << std::endl;

    connect(Network::get_any_interface());
    negociate();
    m_isEof = false;
  }


  bool eof(){ return m_isEof; }

void parse_url(const std::string url, String& server, String& port, String& filename)
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
  
  tmp = tmp.substr(serverb+1);
  if( tmp.find("?") != std::string::npos )
  {
    serverb  = tmp.find("?");
    filename = tmp.substr(0,serverb);
    std::string options  = tmp.substr(serverb+1);
    std::cout << "OPTIONS: " << options << std::endl;
  }else{
    filename = tmp;
  }  
}

  virtual void closef(){
      std::cout << "closing files" << std::endl;
      int32_t cmd=-1;
      *m_writer << cmd;
  }

  void connect(pInterfaceIP interface){
    if( interface == NULL ) MTHROW("No valid interface to host");
    std::cout << "DNFP-Message: Connecting to:"+m_serverip+":"+m_serverport;
    std::cout << " from interface:"+interface->name() << std::endl;
    pConnexion connexion= interface->connect_to( m_serverip, m_serverport );
    m_reader = new Data_reader_socket( connexion->socket() );
    m_writer = new Data_writer_socket( connexion->socket() );
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

  int do_get_bytes(size_t readb, char *buffer){
    if (buffer == NULL) {
      size_t buff_size = 1000000;
      readb = (readb < buff_size ? readb : buff_size);
      char buff[(int)readb];
      
      //int32_t readbyte = readb;
      //*m_writer << readbyte;
      int ret = m_breader->get_bytes(readb, buff);
      //uint32_t eof;
      //*m_breader >> eof;
      //if( eof == 1 ) m_isEof = 1;
      return ret;
    }
    //int32_t readbyte = readb;
    //*m_writer << readbyte;
    int ret = m_breader->get_bytes(readb, buffer);
    //uint32_t eof;
    //*m_breader >> eof;
    //if( eof == 1 ) m_isEof = 1;
    return ret;
  }

  private:
    String m_filename;
    String m_serverip;
    String m_serverport;
    String m_url;

    bool m_isEof;

    Data_writer_socket* m_writer;
    Data_reader_socket* m_reader;
    Data_reader_blocking* m_breader;
};


#endif // DATA_READER_DNFP_H
