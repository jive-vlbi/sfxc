#ifndef CORRELATOR_TIME_H
#define CORRELATOR_TIME_H

#include <stdint.h>
#include <math.h>
#include <iostream>
#include "utils.h"

#define REFERENCE_MJD (51544)
#define SECONDS_PER_DAY (86400)

class Time{
public:
  Time();
  Time(int mjd, double sec, double sample_rate_ = MAX_SAMPLE_RATE);
  Time(double usec, double sample_rate_ = MAX_SAMPLE_RATE);
  Time(const std::string &time);

  void set_sample_rate(double sample_rate_);
  int64_t get_clock_ticks() const;
  void set_clock_ticks(int64_t ticks);

  void set_time(int mjd, double sec);
  void set_time_usec(int mjd, double usec);
  double get_mjd() const;
  double get_time_usec() const;
  double get_time() const;
  void get_date(int &year, int &day) const;
  void get_time(int &h, int &m, int &s, int &ms) const;
  std::string date_string() const;

  static const Time max_time(); 
  double diff(const Time &other) const;
  int64_t diff_samples(const Time &other) const; 

  void inc_samples(int64_t samples);
  void inc_time_usec(int64_t usec);

  void operator=(const Time &other);
  void operator+=(const Time &other);
  void operator-=(const Time &other);

  Time operator+(const Time &other) const;
  Time operator-(const Time &other) const;

  double operator/(const Time &other) const;
  Time operator/(const double &val) const;
  Time operator*(const double &val) const;
  Time operator%(const Time &val) const;

  bool operator==(const Time &other) const;
  bool operator!=(const Time &other) const;
  bool operator<(const Time &other) const;
  bool operator<=(const Time &other) const;
  bool operator>(const Time &other) const;
  bool operator>=(const Time &other) const;
private:
  /// The current time represented as number of clock ticks since reference time
  int64_t nticks;
  /// The number of clock ticks per second (data type is double to avoid int->double conversions)
  const double clock_rate;
  /// The sample rate expressed as the number of clock ticks per sample
  int sample_rate;
};

std::ostream& operator<<(std::ostream &o, const Time t);

inline void Time::set_time(int mjd, double sec){
  nticks = (int64_t)((mjd - REFERENCE_MJD) * SECONDS_PER_DAY * clock_rate) + (int64_t)round(sec * clock_rate);
}

inline void Time::set_time_usec(int mjd, double usec){
  nticks = (int64_t)((mjd - REFERENCE_MJD) * SECONDS_PER_DAY * clock_rate) +
           (int64_t)round(usec * (clock_rate / 1000000));
}

inline int64_t Time::diff_samples(const Time &other) const{
  return (nticks - other.nticks) / sample_rate;
}

inline void Time::inc_samples(int64_t samples){
  nticks += samples * sample_rate;
}

inline void Time::inc_time_usec(int64_t usec){
  nticks += (int64_t)round(usec * clock_rate / 1000000);
}

inline void Time::operator=(const Time &other){
  sample_rate = other.sample_rate;
  nticks = other.nticks;
}

inline bool Time::operator==(const Time &other) const{
  return nticks == other.nticks;
}

inline bool Time::operator!=(const Time &other) const{
  // nb: (sample_rate/other.sample_rate) is exactly representable
  return nticks != other.nticks;
}

inline Time Time::operator+(const Time &other) const{
  Time ret(*this);
  ret.nticks += other.nticks;
  return ret;
}

inline Time Time::operator-(const Time &other) const{
  Time ret(*this);
  ret.nticks -= other.nticks;
  return ret;
}

inline Time Time::operator%(const Time &other) const{
  Time ret(*this);
  ret.nticks %= other.nticks;
  return ret;
}

inline double Time::operator/(const Time &other) const{
  return (double)nticks/other.nticks;
}

inline Time Time::operator/(const double &val) const{
  Time ret(*this);
  ret.nticks = (int64_t)round(ret.nticks / val);
  return ret;
}

inline Time Time::operator*(const double &val) const{
  Time ret(*this);
  ret.nticks = (int64_t)round(ret.nticks * val);
  return ret;
}

inline void Time::operator+=(const Time &other){
  nticks += other.nticks ;
}

inline void Time::operator-=(const Time &other){
  nticks -= other.nticks ;
}

inline bool Time::operator>(const Time &other) const{
  return  nticks > other.nticks;
}

inline bool Time::operator>=(const Time &other) const{
  return nticks >= other.nticks;
}

inline bool Time::operator<(const Time &other) const{
  return nticks < other.nticks;
}

inline bool Time::operator<=(const Time &other) const{
  return nticks <= other.nticks;
}
#endif
