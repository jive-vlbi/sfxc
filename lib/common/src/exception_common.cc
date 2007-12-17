#include <iostream>
#include <vector>
#include "exception_common.h"
#include "demangler.h"

Exception::Exception(const std::string& message, Backtrace& bt) :
        backtrace_(bt)
{
    message_ = message;
}

Exception::~Exception() throw()
{
    //Backtrace::dump();
};

Backtrace& Exception::backtrace()
{
    return backtrace_;
}

const std::string Exception::type()
{
    return get_type_name( this ) ;
}

const std::string Exception::message()
{
    return message_;
}

std::ostream& operator<<(std::ostream& out, Exception& exception)
{
    //out << "============= \033[31m An exception occurs \033[30m  ================ " << std::endl;
    out << "    type: \033[32m" <<  exception.type()      << "\033[30m" << std::endl;
    out << " message: \033[32m" <<  exception.message()   << "\033[30m" << std::endl;
    out << "   trace: " <<  std::endl << exception.backtrace() << "" <<std::endl;

    return out;
}
