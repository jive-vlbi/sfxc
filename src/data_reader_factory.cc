#include "exception_common.h"

#include "data_reader_factory.h"
#include "data_reader_file.h"
#include "data_reader_mark5.h"
#include "data_reader_dnfp.h"

//#include "data_reader_http.h"
//#include "data_reader_ftp.h"

Data_reader* Data_reader_factory::get_reader(const std::string& url)
{
  if ( url.find("file://") == 0 ) {
    //std::cout  << "the stream is in a file" << std::endl;
    return new Data_reader_file(url);
  } else if ( url.find("dnfp://") == 0 ) {
    //std::cout  << "the stream is on an dnfp server" << std::endl;
    return new Data_reader_dnfp(url);
//   } else if ( url.find("mark5://") == 0 ) {
//     std::cout  << "the stream is on an mark5 server" << std::endl;
//     return new Data_reader_mark5(url);
  }

  MTHROW( "No data reader to handle :"+url );
}

