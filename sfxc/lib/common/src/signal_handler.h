#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "singleton.h"

class Signal_handler
{
public:
    static void handler(int s)
    {
        std::cerr << "(S): Signal handler received:" << s << std::endl;
        exit(0);
    }

    static void install()
    {
/*
        if ( signal( SIGSEGV, handler ) == SIG_ERR )
        {
          Log_writer& logger = singleton<Log_writer_cout>::instance();
          LOG2(logger, "(S): Unable to setup handler" );
          exit(0);
        }
         if ( signal( SEGV_MAPERR, handler ) == SIG_ERR )
        {
          Log_writer& logger = singleton<Log_writer_cout>::instance();
          LOG2(logger, "(S): Unable to setup handler" );
          exit(0);
        }
        */
    }
};



#endif // SIGNAL_HANDLER_H
