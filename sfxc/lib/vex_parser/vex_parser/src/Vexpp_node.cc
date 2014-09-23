#include <assert.h>
#include <algorithm>
#include <math.h>
#include <stdio.h>

#include "vex/Vex++.h"

/**
 *  Vexpp_node : Node of the C++ vex tree
 **/
Vexpp_node::Vexpp_node() : m_type(EMPTY) {}

Vexpp_node::Vexpp_node(Type _type) {
  m_type = _type;
}

Vexpp_node::Vexpp_node(const char* ident) {
  assert(ident != NULL);
  m_type = STRING;
  name = ident;

  if (name[0] == '"') {
    name.erase(0, 1);
    name.erase(name.size() - 1, 1);
  }
}

std::ostream &operator<<(std::ostream &out, const Vexpp_node& data) {
  data.print(out, 0);
  out << std::endl;
  return out;
}

std::string escape(std::string const &str, char const *series) {
  std::string ret;
  int left = 0;

  while (true) {
    int right = str.find_first_of(series, left);

    if ((right >= 0) && (right < (int)str.size())) {
      ret += str.substr(left, right - left);
      ret += "\\";
      ret += str[right];
      left = right + 1;
    } else {
      ret += str.substr(left, str.size());
      return ret;
    }
  }
}

std::ostream &Vexpp_node::print(std::ostream &out, int indent) const {
  int indent_lvl = 2;

  switch (m_type) {
  case Vexpp_node::EMPTY: {
      for (int i=0; i<indent; i++) out << " ";
      out << '"' << '"';
      break;
    }
  case Vexpp_node::STRING: {
      for (int i=0; i<indent; i++) out << " ";
      out << '"' << escape(name, "\"") << '"';
      break;
    }
  case Vexpp_node::ARRAY: {
      for (int i=0; i<indent; i++) out << " ";
      out << "[" << std::endl;
      for (const_array_iterator it = lst.begin(); it != lst.end(); it++) {
        (*it).print(out, indent + indent_lvl);
        const_array_iterator next = it;
        next++;
        if (next != lst.end()) {
          out << ",";
        }
        out << std::endl;
      }
      for (int i=0; i<indent; i++) out << " ";
      out << "]";
      break;
    }
  case Vexpp_node::DICT: {
      for (int i=0; i<indent; i++) out << " ";
      out << "{" << std::endl;
      for (const_dict_iterator it = dict.begin(); it != dict.end(); ++it) {
        for (int i=0; i<indent; i++) out << " ";
        out << '"' << it->first << '"' << ": " << std::endl;
        (*it).second.print(out, indent+indent_lvl);
        const_dict_iterator next = it;
        ++next;
        if (next != dict.end()) {
          out << ",";
        }
        out << std::endl;
      }
      for (int i=0; i<indent; i++) out << " ";
      out << "}";
      break;
    }
  }

  return out;
}


size_t
Vexpp_node::size() {
  if (m_type == DICT) {
    return dict.size();
  } else if (m_type == ARRAY) {
    return lst.size();
  }
  return 0;
}

Vexpp_node::iterator
Vexpp_node::operator[](const std::string &key) {
  assert(m_type == DICT);
  dict_iterator it = dict.lower_bound(key);
  if (it != dict.end() && it->first == key) {
    return iterator(this, it);
  }
  return iterator(this, dict.end());
}

Vexpp_node::iterator
Vexpp_node::operator[](size_t i) {
  assert(m_type == ARRAY);
  if (i < lst.size()) {
    return iterator(&lst[i]);
  }
  return iterator(this, lst.end());
}

Vexpp_node::const_iterator
Vexpp_node::operator[](const std::string &key) const {
  assert(m_type == DICT);
  const_dict_iterator it = dict.lower_bound(key);
  if (it != dict.end() && it->first == key) {
    return const_iterator(this, it);
  }
  return const_iterator(this, dict.end());
}

Vexpp_node::const_iterator
Vexpp_node::operator[](size_t i) const {
  assert(m_type == ARRAY);
  if (i < lst.size()) {
    return const_iterator(&lst[i]);
  }
  return const_iterator(this, lst.end());
}

void Vexpp_node::set_key(const std::string &key, Self& value) {
  assert(m_type == DICT);
  dict.insert(dict_element(key, value));
}

void Vexpp_node::append(const Self& value) {
  assert(m_type == ARRAY);
  lst.push_back(value);
}

void Vexpp_node::join(Self& value) {
  assert(m_type == DICT);
  assert(value.m_type == DICT);
  assert(value.size() == 1);

  iterator it = value.begin();
  set_key(it.key(), *it);
}

// Convertors
std::string Vexpp_node::dimension(const std::string &unit) const {
  if (unit == "um" || unit == "mm" || unit == "cm" || unit == "m" ||
      unit == "km")
    return "length";

  if (unit == "psec" || unit == "nsec" || unit == "usec" || unit == "msec" ||
      unit == "sec" || unit == "min" || unit == "hr" || unit == "yr")
    return "time";

  if (unit == "mHz" || unit == "Hz" || unit == "kHz" || unit == "MHz" ||
      unit == "GHz")
    return "frequency";

  if (unit.find("/")) {
    std::string unit1 = unit.substr(0, unit.find("/"));
    std::string unit2 = unit.substr(unit.find("/") + 1);
    if (dimension(unit1) == "length" && dimension(unit2) == "time")
      return "velocity";
  }

  return std::string();
}

double Vexpp_node::scale(const std::string &unit) const {
  if (unit == "psec")
    return 1e-12;
  if (unit == "nsec")
    return 1e-9;
  if (unit == "um" || unit == "usec")
    return 1e-6;
  if (unit == "mm" || unit == "msec" || unit == "mHz")
    return 1e-3;
  if (unit == "cm")
    return 1e-2;
  if (unit == "m" || unit == "sec" || unit == "Hz")
    return 1;
  if (unit == "km" || unit == "kHz")
    return 1e3;
  if (unit == "MHz")
    return 1e6;
  if (unit == "GHz")
    return 1e9;

  if (unit == "min")
    return 60;
  if (unit == "hr")
    return 3600;
  if (unit == "yr")
    return 365.25 * 86400;

  if (unit.find("/")) {
    std::string unit1 = unit.substr(0, unit.find("/"));
    std::string unit2 = unit.substr(unit.find("/") + 1);
    return scale(unit1) / scale(unit2);
  }

  return NAN;
}

int Vexpp_node::to_int() const {
  assert(m_type == STRING);
  int result, err;
  err = sscanf(name.c_str(), "%d", &result);
  assert(err==1);
  return result;
}

int Vexpp_node::to_int_amount(const std::string &unit) const {
  assert(m_type == STRING);
  int result, err;
  char unit2[name.size()];
  err = sscanf(name.c_str(), "%d %s", &result, unit2);
  assert(err==2);
  if (unit != unit2) {
    assert(dimension(unit) != std::string());
    assert(dimension(unit) == dimension(unit2));
    return result * scale(unit2) / scale(unit);
  }

  return result;
}

double Vexpp_node::to_double() const {
  assert(m_type == STRING);
  double result;
  int err;
  err = sscanf(name.c_str(), "%lf", &result);
  assert(err==1);
  return result;
}

double Vexpp_node::to_double_amount(const std::string &unit) const {
  assert(m_type == STRING);
  double result;
  int err;
  char unit2[name.size()];
  err = sscanf(name.c_str(), "%lf %s", &result, unit2);
  assert(err==2);
  if (unit != unit2) {
    assert(dimension(unit) != std::string());
    assert(dimension(unit) == dimension(unit2));
    return result * scale(unit2) / scale(unit);
  }

  return result;
}

char Vexpp_node::to_char() const {
  assert(m_type == STRING);
  return name[0];
}

Vexpp_node::Date Vexpp_node::to_date() const {
  return Date(name);
}

std::string Vexpp_node::to_string() const {
  assert(m_type == STRING);
  return name;
}

const char *Vexpp_node::to_c_string() const {
  assert(m_type == STRING);
  return name.c_str();
}


Vexpp_node::iterator Vexpp_node::begin() {
  assert(m_type == ARRAY || m_type == DICT);
  if (m_type == ARRAY) {
    return iterator(this, lst.begin());
  } else {
    return iterator(this, dict.begin());
  }
}

Vexpp_node::iterator Vexpp_node::end() {
  assert(m_type == ARRAY || m_type == DICT);
  if (m_type == ARRAY) {
    return iterator(this, lst.end());
  } else {
    return iterator(this, dict.end());
  }
}

Vexpp_node::const_iterator Vexpp_node::begin() const {
  assert(m_type == ARRAY || m_type == DICT);
  if (m_type == ARRAY) {
    return const_iterator(this, lst.begin());
  } else {
    return const_iterator(this, dict.begin());
  }
}

Vexpp_node::const_iterator Vexpp_node::end() const {
  assert(m_type == ARRAY || m_type == DICT);
  if (m_type == ARRAY) {
    return const_iterator(this, lst.end());
  } else {
    return const_iterator(this, dict.end());
  }
}

Vexpp_node::iterator
Vexpp_node::begin(const std::string &key) {
  assert(m_type == DICT);
  dict_iterator it = dict.lower_bound(key);
  if (it != dict.end() && it->first == key) {
    return iterator(this, it);
  }
  return iterator(this, dict.end());
}

Vexpp_node::iterator
Vexpp_node::end(const std::string &key) {
  assert(m_type == DICT);
  if (dict.find(key) == dict.end())
    return iterator(this, dict.end()); // key not present in dictionary
  return iterator(this, dict.upper_bound(key));
}

Vexpp_node::iterator
Vexpp_node::begin_key(const char *key) {
  Vexpp_node::iterator result = begin(key);
  return result;
}

Vexpp_node::iterator
Vexpp_node::end_key(const char *key) {
  Vexpp_node::iterator result = end(key);
  return result;
}

Vexpp_node::const_iterator Vexpp_node::begin(const std::string &key) const {
  assert(m_type == DICT);
  const_dict_iterator it = dict.lower_bound(key);
  if (it != dict.end() && it->first == key) {
    return const_iterator(this, it);
  }
  return const_iterator(this, dict.end());
}

Vexpp_node::const_iterator Vexpp_node::end(const std::string &key) const {
  assert(m_type == DICT);
  if (dict.find(key) == dict.end())
    return const_iterator(this, dict.end()); // key not present in dictionary
  return const_iterator(this, dict.upper_bound(key));
}

size_t Vexpp_node::size() const {
  if (m_type == ARRAY) {
    return lst.size();
  } else {
    return dict.size();
  }
  assert((m_type == ARRAY) || (m_type == DICT));
}

Vexpp_node::Type Vexpp_node::get_type() {
  return m_type;
}

const char *
Vexpp_node::type() {
  switch (m_type) {
  case STRING:
    return "STRING";
  case ARRAY:
    return "ARRAY";
  case DICT:
    return "DICT";
  default: {
      assert (m_type==EMPTY);
      return "EMPTY";
    }
  }
  return "empty";
}

/* Date members */
Vexpp_node::Date::Date() : year(0), day(0), hour(0), minute(0), second(0) {}

Vexpp_node::Date::Date(const std::string &date) :
    year(0), day(0), hour(0), minute(0), second(0) {
  sscanf(date.c_str(), "%dy%dd%dh%dm%ds",
         &year, &day, &hour, &minute, &second);
}

Vexpp_node::Date::Date(int year, int day, int seconds) :
    year(year), day(day) {
  second = seconds % 60;
  seconds /= 60;
  minute = seconds % 60;
  seconds /= 60;
  hour   = seconds;

  assert(hour <= 23);
}

Vexpp_node::Date
Vexpp_node::Date::operator+(const Date &other) const {
  Date result;
  result.year   = year + other.year;
  result.day    = day + other.day;
  result.hour   = hour + other.hour;
  result.minute = minute + other.minute;
  result.second = second + other.second;
  result.normalize();
  return result;
}

bool
Vexpp_node::Date::operator<(const Date &other) const {
  Date left = *this;
  left.normalize();
  Date right = other;
  right.normalize();
  if (left.year < right.year)  return true;
  if (left.year != right.year) return false;
  if (left.day < right.day)    return true;
  if (left.day != right.day)   return false;
  if (left.hour < right.hour)  return true;
  if (left.hour != right.hour) return false;
  if (left.minute < right.minute)  return true;
  if (left.minute != right.minute) return false;
  if (left.second < right.second)  return true;
  return false;
}

bool
Vexpp_node::Date::operator==(const Date &other) const {
  Date left = *this;
  left.normalize();
  Date right = other;
  right.normalize();
  return ((left.year == right.year) &&
          (left.day == right.day) &&
          (left.hour == right.hour) &&
          (left.minute == right.minute) &&
          (left.second == right.second));
}

bool
Vexpp_node::Date::operator<=(const Date &other) const {
  return (*this < other) || (*this == other);
}

bool
Vexpp_node::Date::operator>=(const Date &other) const {
  return (other < *this) || (*this == other);
}

bool
Vexpp_node::Date::operator>(const Date &other) const {
  return (other < *this);
}

std::string
Vexpp_node::Date::to_string() const {
  char date[20];
  snprintf(date, 20, "%04dy%03dd%02dh%02dm%02ds",
           year, day, hour, minute, second);
  return std::string(date);
}

void
Vexpp_node::Date::normalize() {
  minute += second/60;
  second = second%60;
  hour   += minute/60;
  minute = minute%60;
  day    += hour/24;
  hour = hour%24;
  int days_per_year = 365;
  if ((year % 4 == 0) && ((year % 100 != 0) && (year % 400 == 0))) {
    days_per_year = 366;
  }
  year += day/days_per_year;
  day = day%days_per_year;
}

// Keys:
Vexpp_node::const_key_iterator
Vexpp_node::keys_begin() const {
  assert(m_type == DICT);
  return const_key_iterator(dict.begin(), dict.end());
}
Vexpp_node::const_key_iterator
Vexpp_node::keys_end() const {
  assert(m_type == DICT);
  return const_key_iterator(dict.end(), dict.end());
}

Vexpp_node::const_key_iterator::const_key_iterator(const_dict_iterator curr,
    const_dict_iterator end)
    : m_curr(curr), m_end(end) {}
Vexpp_node::const_key_iterator
Vexpp_node::const_key_iterator::operator++() {
  assert(m_curr != m_end);
  const_dict_iterator first = m_curr;
  while ((m_curr != m_end) &&
         (first->first == m_curr->first)) {
    m_curr++;
  }
  return *this;
}
Vexpp_node::const_key_iterator
Vexpp_node::const_key_iterator::operator++(int) {
  Vexpp_node::const_key_iterator self = *this;
  assert(m_curr != m_end);
  while ((m_curr != m_end) &&
         (self.m_curr->first == m_curr->first)) {
    m_curr++;
  }
  return self;
}
bool
Vexpp_node::const_key_iterator::operator==(const_key_iterator &other) {
  return m_curr == other.m_curr;
}
bool
Vexpp_node::const_key_iterator::operator!=(const_key_iterator &other) {
  return m_curr != other.m_curr;
}
std::string
Vexpp_node::const_key_iterator::operator*() const {
  assert(m_curr != m_end);
  return m_curr->first;
}
