#include "monitor.h"
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <sstream>

// Those two are needed for getusec()
# include <unistd.h>
#include <time.h>
# include <sys/time.h>
//#include "tools.hh"
#include "exception_common.h"

double toMB(unsigned int size) {
  return (1.0*size)/(1024*1024);
}


void getusec(uint64_t &utime) {
  struct timeval tv;
  gettimeofday(&tv,0);
  utime=tv.tv_sec*1000000;
  utime+=tv.tv_usec;
}

uint64_t ticksPerSec(void) {
  return 1000000;
}

uint64_t ticksPerMSec(void) {
  return 1000000/1000;
}


double tickToSec(uint64_t ticks) {
  return 1.0*ticks/ticksPerSec();
}


////////////////////////////////////////////////////////////////////////
//
// QOS_Monitor
//
////////////////////////////////////////////////////////////////////////
QOS_Monitor::QOS_Monitor() {
  m_name = "Unnamed";
  m_samples_interval = 0;
  m_history_size = 0;
}

QOS_Monitor::~QOS_Monitor() {}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed::SampleSpeed& sample) {
  out << toMB( sample.get_bytecount() )/tickToSec(sample.get_duration()) << " MB/s";
  return out;
}


void QOS_Monitor::set_name(const std::string& name) {
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
QOS_MonitorSpeed::QOS_MonitorSpeed() {
  inited_ = false;
}

QOS_MonitorSpeed::QOS_MonitorSpeed(const std::string& name, const int interval_ms, const std::string& dirname, int history_size) {
  inited_ = false;
  init(name, interval_ms, dirname, history_size);
}

QOS_MonitorSpeed::~QOS_MonitorSpeed() {
  // The monitor correctly initiated ?
  if ( inited_ ) {
    // Finalize by saving the last remaining samples
    for (unsigned int i=0;i<current_sample_;i++) {
      fout_ << m_history[i].toString();
    }
    fout_.close();

    // Write the meta-data related to this monitor
    std::stringstream filename;
    filename << filenamebase_.str() << ".meta";

    fout_.open(filename.str().c_str());
    if ( fout_.fail() ) {
      MTHROW("Unable to open file: "+filename.str());
    }
    fout_ << "[" << std::endl;

    fout_ << "['" << m_name << "' , 'is_a', 'monitor_speed']" << std::endl;
    fout_ << ",['" << m_name << "' , 'location', '" << filenamebase_.str() << ".data']" << std::endl;

    for (unsigned int i=0;i<properties_.size();i++) {
      fout_ << "," << properties_[i] << std::endl;
    }

    fout_ << "]" << std::endl;
    fout_.close();
  }


}

void QOS_MonitorSpeed::init(const std::string& name, const int sampling_interval_ms,  const std::string& dirname, int history_size) {
  //assert( inited_ == false && "Double initialization !");
  if ( inited_ == true ) MTHROW("Double Initialization");

  m_is_measuring = false;

  current_sample_ = 0;
  m_name = name;
  m_history.resize(history_size);

  sampling_interval_ticks_ = (sampling_interval_ms*ticksPerMSec());

  // Initialize the base par of the output filename
  filenamebase_.str("");
  filenamebase_ << dirname << "/" << m_name << "_XXXXXX";
  char writeablestr[ filenamebase_.str().size()+1 ];
  strcpy(writeablestr, filenamebase_.str().c_str() );
  mkstemp(writeablestr);
  filenamebase_.str(writeablestr);

  // We open the file in which write the monitoring data
  std::stringstream filename;
  filename << filenamebase_.str() << ".data";
  std::cout << " Writing to : " << filename.str() << std::endl;
  fout_.open(filename.str().c_str());
  if ( fout_.fail() ) {
    MTHROW("Unable to open file: "+filename.str());
  }

  inited_ = true;

  for (unsigned int i=0;i<m_history.size();i++) {
    m_history[i].reset();
  }
}


void QOS_MonitorSpeed::begin_measure() {
  assert( inited_ && "Not correctly initialized" );
  assert( !m_is_measuring && "you cannot start a measure before finished the previous one" );

  if ( !m_is_measuring ) {
    m_is_measuring = true;
    getusec(m_begin_time);
  }
}

void QOS_MonitorSpeed::finalize_current_sample() {
  current_sample_++;
  if ( current_sample_ == m_history.size() ) {
    for (unsigned int i=0;i<m_history.size();i++) {
      fout_ << m_history[i].toString();
      m_history[i].reset();
    }
    current_sample_ = 0;
  }
}

void QOS_MonitorSpeed::end_measure(uint64_t bytecount) {
  assert( m_is_measuring && "attempt to finalize a measure while no measurement was started" );
  if ( inited_ ) {
    m_is_measuring = false;
    getusec(m_end_time);

    uint64_t length = m_end_time - m_begin_time;
    // check if the sub sampling avreaging is enabled
    if ( sampling_interval_ticks_ > 0 && length < sampling_interval_ticks_ ) {
      uint64_t totallength = m_history[current_sample_].get_duration();
      uint64_t diff   = sampling_interval_ticks_ - totallength;

      //std::cout << "totallength:" << totallength <<" diff:"<<diff << " sampling_interval:" << sampling_interval_ticks_ << std::endl;

      // Do we need to split the interval in two
      if ( totallength + length < sampling_interval_ticks_ ) {
        // as we have not reach the sample limite the value are added
        // to the current sample
        m_history[current_sample_].add_subsample(m_begin_time, m_end_time, bytecount);
        //std::cout << "PATH A: " << current_sample_ << " totallenght:"<< totallength << std::endl;
      } else {
        //std::cout << "PATH B:" << current_sample_ << std::endl;
        // linear interpolation of the value
        if ( totallength < sampling_interval_ticks_ ) {
          double alpha = ((1.0*(length-diff))*bytecount)/length;
          uint64_t part1 = alpha;
          uint64_t part2 = bytecount-part1;

          // Save the previous sample._t
          m_history[current_sample_].add_subsample(m_begin_time, m_begin_time+diff, part1);
          finalize_current_sample();

          // Initialize the next sample
          m_history[current_sample_].set(m_begin_time, m_end_time, part2);
        } else {
          // Save the previous sample._t
          finalize_current_sample();

          // Initialize the next sample
          m_history[current_sample_].set(m_begin_time, m_end_time, bytecount);

        }
      }
    }

    // Not sampling limit, we have to record all the samples.
    // Beware this may generate a lot of data.
    else {
      m_history[current_sample_].set(m_begin_time, m_end_time, bytecount);
      finalize_current_sample();
    }
  }
}

std::string QOS_MonitorSpeed::toString() {
  std::stringstream out;

  uint64_t numsamples = m_history.size();
  uint64_t bytecount=0;
  uint64_t timecount=0;

  for (unsigned int i=0; i< m_history.size();i++) {
    timecount += m_history[i].get_duration();
    bytecount += m_history[i].get_bytecount();
  }

  out << "Monitor[" << m_name << "]:" <<  std::endl;
  out << "      " << numsamples << " samples collected" << std::endl;
  out << "      " << bytecount << " bytes transmitted in " << tickToSec(timecount) << " seconds" << std::endl;

  return out.str();
}

void QOS_MonitorSpeed::save_history_to_file(const std::string& dirname) {
  std::stringstream filename;
  filename << dirname << "/" << m_name << ".data";
  std::ofstream fout(filename.str().c_str());

  for (unsigned int i=0; i< m_history.size();i++) {
    fout << m_history[i].toString() << std::endl;
  }


  return;
}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed& mon) {
  return out << mon.toString();
}

std::ostream& operator<<(std::ostream& out, QOS_MonitorSpeed* mon) {
  return out<<*mon;
}

#ifdef ENABLE_TEST_UNIT
void QOS_MonitorSpeed::Test::tests() {
  QOS_MonitorSpeed monitor;

  TEST_EXCEPTION_NTHROW(  monitor.init("test_monitor", 50, "./", 1000) );
  TEST_EXCEPTION_THROW(  monitor.init("This one should fails") );

  //monitor.init();
  std::cout << "Testing subsampling" << std::endl;
  for (unsigned int i=0;i<100;i++) {
    monitor.begin_measure();
    usleep( 100000*(1.0*random()/RAND_MAX) );
    monitor.end_measure(1);
    usleep( 100000*(1.0*random()/RAND_MAX) );
  }

  std::cout << "Testing over-sampling" << std::endl;
  for (unsigned int i=0;i<10; i++) {
    monitor.begin_measure();
    sleep(3*(1.0*random()/RAND_MAX));
    monitor.end_measure(1);
    sleep(3*(1.0*random()/RAND_MAX));
  }

  std::cout << "Testing subsampling (again)" << std::endl;
  for (unsigned int i=0;i<100;i++) {
    monitor.begin_measure();
    usleep( 100000*(1.0*random()/RAND_MAX) );
    monitor.end_measure(1);
    usleep( 100000*(1.0*random()/RAND_MAX) );
  }

}
#endif // ENABLE_TEST_UNIT



////////////////////////////////////////////////////////////////////////
//
// QOS_MonitorLatency
//
////////////////////////////////////////////////////////////////////////
QOS_MonitorLatency::QOS_MonitorLatency() {}

QOS_MonitorLatency::~QOS_MonitorLatency() {}

void QOS_MonitorLatency::begin_measure() {}

void QOS_MonitorLatency::end_measure() {}

////////////////////////////////////////////////////////////////////////
//
// QOS_MonitorState
//
////////////////////////////////////////////////////////////////////////
