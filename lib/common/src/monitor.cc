#include "monitor.h"

#include <cassert>
#include <iostream>
#include <sstream>

// Those two are needed for getusec()
# include <unistd.h>
#include <time.h>
# include <sys/time.h>
//#include "tools.hh"

double toMB(unsigned int size)
{
    return (1.0*size)/(1024*1024);
}


void getusec(uint64_t &utime){
    struct timeval tv;
	gettimeofday(&tv,0);
	utime=tv.tv_sec*1000000;
	utime+=tv.tv_usec;
}

uint64_t ticksPerSec(void)
{
  return 1000000;
}

double tickToSec(uint64_t ticks)
{
    return 1.0*ticks/ticksPerSec();
}


////////////////////////////////////////////////////////////////////////
//
// QOS_Monitor
//
////////////////////////////////////////////////////////////////////////
QOS_Monitor::QOS_Monitor()
{
    m_name = "Unnamed";
    m_samples_interval = 0;
    m_history_size = 0;
}

QOS_Monitor::~QOS_Monitor()
{

}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed::SampleSpeed& sample)
{
      out << toMB( sample.get_bytecount() )/tickToSec(sample.get_duration()) << " MB/s";
return out;
}

void QOS_Monitor::set_name(const std::string& name)
{
    m_name =  name;
}

/*
void QOS_Monitor::set_history_size(unsigned int numsamples)
{
    m_history_size = numsamples;
}

void QOS_Monitor::set_sampling_control(unsigned int interval)
{
    m_samples_interval = interval;
}
*/

////////////////////////////////////////////////////////////////////////
//
// QOS_MonitorSpeed
//
////////////////////////////////////////////////////////////////////////
QOS_MonitorSpeed::QOS_MonitorSpeed()
{
    m_is_measuring = false;
}

QOS_MonitorSpeed::~QOS_MonitorSpeed()
{

}

void QOS_MonitorSpeed::begin_measure()
{
    assert( !m_is_measuring && "you cannot start a measure before finished the previous one" );

    m_is_measuring = true;

    //time(&m_begin_time);
    getusec(m_begin_time);
}

void QOS_MonitorSpeed::end_measure(uint64_t bytecount)
{
    assert( m_is_measuring && "attempt to finalize a measure while no measurement was started" );

    m_is_measuring = false;
    getusec(m_end_time);
    m_history.push_back( SampleSpeed(m_begin_time, m_end_time, bytecount) );
}

std::string QOS_MonitorSpeed::toString()
{
    std::stringstream out;

    uint64_t numsamples = m_history.size();
    uint64_t bytecount=0;
    uint64_t timecount=0;

    for(unsigned int i=0; i< m_history.size();i++)
    {
        timecount += m_history[i].get_duration();
        bytecount += m_history[i].get_bytecount();
    }

    out << "Monitor[" << m_name << "]:" <<  std::endl;
    out << "      " << numsamples << " samples collected" << std::endl;
    out << "      " << bytecount << " bytes transmitted in " << tickToSec(timecount) << " seconds" << std::endl;

    return out.str();
}

void QOS_MonitorSpeed::save_history_to_file(const std::string& dirname)
{
    std::stringstream filename;
    filename << dirname << "/" << m_name << ".data";
    std::ofstream fout(filename.str().c_str());

    for(unsigned int i=0; i< m_history.size();i++)
    {
        fout << m_history[i].toString() << std::endl;
    }


    return;
}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed& mon)
{
    return out << mon.toString();
}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed* mon)
{
    return out<<*mon;
}

////////////////////////////////////////////////////////////////////////
//
// QOS_MonitorLatency
//
////////////////////////////////////////////////////////////////////////
QOS_MonitorLatency::QOS_MonitorLatency()
{

}

QOS_MonitorLatency::~QOS_MonitorLatency()
{

}

void QOS_MonitorLatency::begin_measure()
{

}

void QOS_MonitorLatency::end_measure()
{

}

////////////////////////////////////////////////////////////////////////
//
// QOS_MonitorState
//
////////////////////////////////////////////////////////////////////////
