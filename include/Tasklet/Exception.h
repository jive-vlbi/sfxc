#ifndef EXCEPTION_HH_INCLUDED
#define EXCEPTION_HH_INCLUDED


#if !defined(WIN32)
#include <signal.h>
#endif
#if !defined(WIN32) && !defined(__APPLE__)
#include <execinfo.h>
#include <unistd.h>
#endif
#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Backtrace
{
public:
    static void dump()
    {
        void *array[128];
        int size = backtrace(array, sizeof(array) / sizeof(array[0]));
        if (size > 0)
        {
            char** symbols = backtrace_symbols(array, size);
            if (symbols != NULL)
            {
                for (int i = 0; i < size; ++i)
                {
                    char* symbol = symbols[i];
                    std::cerr << symbol << std::endl;
                }
            }
            else
            {
                backtrace_symbols_fd(array, size, STDERR_FILENO);
            }
        }
    }
};

class Exception
{
    std::string m_backtrace;
public:
    Exception()
    {
        Backtrace::dump();
    }

    const std::string& backtrace()
    {
        return m_backtrace;
    }
};

class NamingException : public Exception
{
    std::string m_message;
public:
    NamingException(const std::string& string)
    {}

    const std::string& message()
    {
        return m_message;
    }
};


#endif // EXCEPTION_HH_INCLUDED
