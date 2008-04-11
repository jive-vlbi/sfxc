/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 */
#ifndef QOS_MONITOR_HH
#define QOS_MONITOR_HH

class QOS_Monitor;
class QOS_MonitorSpeed;
class QOS_MonitorLatency;

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>

// Those two are needed for getusec()
# include <unistd.h>
#include <time.h>
# include <sys/time.h>
#include <sstream>

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

#ifdef RUNTIME_STATISTIC
#define RT_STAT(x) x;
#else
#define RT_STAT(x)
#endif //RUNTIME_STATISTIC

/********************************************************
*    Virtual class root of Monitoring and QOS services.
***/
class QOS_Monitor
{
    public:
        QOS_Monitor();
        virtual ~QOS_Monitor();

        void set_name(const std::string& name);

        /********
        * use this function to specify the number of
        * samples to save.
        * 0 means unlimited and the default choice is 0.
        *****/
        //void set_history_size(unsigned int numsamples);

        /********
        * sampling control is a functionnality that
        * reduce the numbers of sample saved.
        * 0 means no sampling control (default).
        * other value are the minimal time between
        * two samples in milliseconds.
        *****/
        //void set_sampling_control(unsigned int ms);

    protected:
        std::string m_name;

        uint64_t m_samples_interval;
        unsigned int m_history_size;
};

/********************************************************
*    Class to monitor a speed
*    the speed is often measured in Byte/seconds
***/
#ifndef RUNTIME_STATISTIC_DIR
#define RUNTIME_STATISTIC_DIR "stats/"
#endif //RUNTIME_STATISTIC_DIR

class QOS_MonitorSpeed : public QOS_Monitor
{
    public:
        QOS_MonitorSpeed();
        QOS_MonitorSpeed(const std::string& name, const int interval_ms=0, const std::string& dirname=RUNTIME_STATISTIC_DIR, int history_size=1000);
        virtual ~QOS_MonitorSpeed();

        void init(const std::string& name, const int interval_ms=200, const std::string& dirname=RUNTIME_STATISTIC_DIR, int history_size=1000);

        template<class I, class J>
        void add_property(const std::string& s, const I& p, const J& v){
          std::stringstream str;
          str << "['" << s << "', '" << p << "', '" << v << "']";
          properties_.push_back(str.str());
         }

        void begin_measure();
        void end_measure(uint64_t bytecount);

        void save_history_to_file(const std::string& filename);

        std::string toString();
        friend std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed&);
        friend std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed*);

        class SampleSpeed
        {
            public:
                SampleSpeed(){
                        m_begin = 0;
                        m_end = 0;
                        m_numbytes = 0;
                        m_numtime = 0;
                }

                SampleSpeed(const SampleSpeed& s){
                    m_begin = s.m_begin;
                    m_end = s.m_end;
                    m_numtime = s.m_numtime;
                    m_numbytes = s.m_numbytes;
                }

                void set(time_t begin, time_t end, uint64_t numbytes){
                        m_begin = begin;
                        m_end = end;
                        m_numtime = end-begin;
                        m_numbytes = numbytes;
                }

                void reset()
                {
                  m_begin = 0;
                }

                void add_subsample(time_t begin, time_t end, uint64_t numbytes){
                  if( m_begin == 0)
                  {
                        m_begin = begin;
                        m_end = end;
                        m_numtime = end-begin;
                        m_numbytes = numbytes;
                  }else{
                        m_end = end;
                        m_numtime += end-begin;
                        m_numbytes += numbytes;
                  }
                }

                uint64_t m_begin;
                uint64_t m_end;
                uint64_t m_numtime;
                uint64_t m_numbytes;

                uint64_t get_bytecount(){ return m_numbytes; }
                uint64_t get_duration(){
                  if( m_begin == 0 ) return 0;
                  return m_end-m_begin;
                }
                uint64_t get_timecount(){ return m_numtime; }

                friend std::ostream& operator<<(std::ostream& out, SampleSpeed&);

                std::string toString()
                {
                        std::stringstream data;
                        data << m_begin << " " << m_end << " " << m_numtime << " "<< m_numbytes << std::endl;
                        return data.str();
                }
        };
        SampleSpeed& last_measure(){ return m_history[ m_history.size()-1 ]; }

        #ifdef ENABLE_TEST_UNIT
        class Test : public Test_aclass< QOS_MonitorSpeed > {
          public:
            void tests();
        };
#endif //ENABLE_TEST_UNIT

    private:
        // PConstructor
        QOS_MonitorSpeed(const QOS_MonitorSpeed& );

        // PFunction
        void finalize_current_sample();

        // Attributes
        unsigned int current_sample_;
        uint64_t m_begin_time;
        uint64_t m_end_time;
        uint64_t sampling_interval_ticks_;

        bool m_is_measuring;
        bool inited_;

        std::ofstream fout_;
        std::vector<SampleSpeed> m_history;
        std::vector<std::string> properties_;
        std::stringstream filenamebase_;
};

/********************************************************
*    class to use for monitoring latency (in msec)
***/

class QOS_MonitorLatency : public QOS_Monitor
{
    public:
        QOS_MonitorLatency();
        virtual ~QOS_MonitorLatency();

        void begin_measure();
        void end_measure();
    private:
        class SampleLatency
        {
            public:
                time_t m_begin;
                time_t m_end;

                void set(time_t begin, time_t end);
                time_t get_diff();
        };

        std::vector<SampleLatency> m_history;
};

template<class T>
class QOS_MonitorState : public QOS_Monitor
{
    public:
        QOS_MonitorState();
        virtual ~QOS_MonitorState();

        void begin_measure();
        void end_measure(T& state);

        void save_history_to_file(const std::string& filename);

        std::string toString();

        template<class J>
        friend std::ostream& operator<<(std::ostream& out, QOS_MonitorState<J>&);

        template<class J>
        friend std::ostream& operator<<(std::ostream& out, QOS_MonitorState<J>*);
    private:
        class SampleState
        {
            public:
                time_t m_begin;
                time_t m_end;

                T m_state;
                SampleState(time_t begin, time_t end, T& state)
                {
                        m_begin = begin;
                        m_end = end;
                        m_state = state;
                }

                void set(time_t begin, time_t end, T& state);

                T get_state(){ return m_state; }
                uint64_t  get_duration(){ return m_end-m_begin; }

                time_t get_diff();

                std::string toString()
                {
                        std::stringstream mstring;
                        mstring << (uint64_t)m_begin << " " << (uint64_t)m_end << " " << m_state;
                        return mstring.str();
                }
        };

    bool m_is_measuring;
     uint64_t m_begin_time;
        uint64_t m_end_time;



        std::vector<SampleState> m_history;
};

template<class T>
QOS_MonitorState<T>::QOS_MonitorState()
{
    m_is_measuring = false;
}

template<class T>
QOS_MonitorState<T>::~QOS_MonitorState()
{

}

template<class T>
void QOS_MonitorState<T>::save_history_to_file(const std::string& dirname)
{
    std::stringstream filename;
    filename << dirname << "/" << m_name << ".data";

    std::ofstream fout( filename.str().c_str()  );
    for(unsigned int i=0; i< m_history.size();i++)
    {
        //timecount += m_history[i].get_duration();
        //bytecount += m_history[i].get_bytecount();
        fout << m_history[i].toString() << std::endl;
    }

    fout.close();
    return;
}

void getusec(uint64_t &utime);
template<class T>
void QOS_MonitorState<T>::begin_measure()
{
    assert( !m_is_measuring && "you cannot start a measure before finished the previous one" );

    m_is_measuring = true;

    getusec(m_begin_time);
}

template<class T>
void QOS_MonitorState<T>::end_measure(T& sfxcstate)
{
    assert( m_is_measuring && "attempt to finalize a measure while no measurement was started" );

    m_is_measuring = false;
    getusec(m_end_time);
    m_history.push_back( SampleState(m_begin_time, m_end_time, sfxcstate) );
}

template<class T>
std::string QOS_MonitorState<T>::toString()
{
    std::stringstream out;

    uint64_t numsamples = m_history.size();
    uint64_t bytecount=0;
    uint64_t timecount=0;

    for(unsigned int i=0; i< m_history.size();i++)
    {
        timecount += m_history[i].get_duration();
        //bytecount += m_history[i].get_bytecount();
    }

    out << "Monitor[" << m_name << "]:" <<  std::endl;
    out << "      " << numsamples << " samples collected" << std::endl;
    //out << "      " << bytecount << " bytes transmitted in " << tickToSec(timecount) << " seconds" << std::endl;

    return out.str();
}

#endif // MONITOR_HH
