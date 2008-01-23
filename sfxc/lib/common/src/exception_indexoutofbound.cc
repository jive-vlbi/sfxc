#include "exception_indexoutofbound.h"
#include "demangler.h"

Exception_indexoutofbound::Exception_indexoutofbound(unsigned int index, unsigned int size, Backtrace& bt) :
        Exception("", bt)
{

    index_ = index;
    size_ = size;

    std::ostringstream stream;
    stream << "Array size[" << size_ << "]" << " while index[" << index_ << "]";
    message_ = stream.str();
}

Exception_indexoutofbound::~Exception_indexoutofbound() throw() {};


const std::string Exception_indexoutofbound::type()
{
    return get_type_name( this ) ;;
}
