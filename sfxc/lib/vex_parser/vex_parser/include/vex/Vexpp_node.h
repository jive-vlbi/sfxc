#ifndef VEXPP_NODE_H
#define VEXPP_NODE_H

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <iostream>

template <class Vexpp_node_iterator_type>
class Vexpp_node_iterator;

class Vexpp_node_iterator_type;
class Vexpp_node_iterator_const_type;

/**
 *  Vexpp_node defines a node in the C++ tree structure representing the
 *  vex-file
 **/
class Vexpp_node {
  template <class Vexpp_node_iterator_type>
  friend class Vexpp_node_iterator;

  typedef Vexpp_node Self;
public:
  enum Type {
    STRING=0, // name is set, next and child are null
    ARRAY,    // name is content, child is null
    DICT,     // name is key, child contains content
    EMPTY     // Empty elements in an array
  };


  // Constructors
  Vexpp_node();
  Vexpp_node(Type type);
  Vexpp_node(const char* ident);

  class Date {
  public:
    Date();
    Date(const std::string &date);
    Date(int year, int day, int second);

    Date operator+(const Date &other) const;
    bool operator<(const Date &other) const;
    bool operator<=(const Date &other) const;
    bool operator==(const Date &other) const;
    bool operator>=(const Date &other) const;
    bool operator>(const Date &other) const;

    std::string to_string() const;
    void normalize();

    int64_t to_miliseconds() {
      return to_miliseconds(day, year);
    }
    int64_t to_miliseconds(int start_day) {
      return to_miliseconds(start_day, year);
    }
    int64_t to_miliseconds(int start_day, int start_year) {
      int days_per_year =
        ((start_year%4==0) &&
         ((start_year%100!=0) || (start_year%400==0)) ? 366 : 365);

      return 1000 * (second +
                     60 * (minute +
                           60 * (hour +
                                 24 * ((day-start_day) +
                                       days_per_year * (year-start_year)))));
    }
    int year, day, hour, minute, second;
  };


  typedef std::vector<Vexpp_node>                Array;
  typedef std::multimap<std::string, Vexpp_node> Dict;
  typedef Array::iterator                        array_iterator;
  typedef Array::const_iterator                  const_array_iterator;
  typedef std::pair<std::string, Vexpp_node>     dict_element;
  typedef Dict::iterator                         dict_iterator;
  typedef Dict::const_iterator                   const_dict_iterator;

  class const_key_iterator {
  public:
    typedef std::string                        value_type;
    typedef std::forward_iterator_tag            iterator_category;
    typedef ptrdiff_t                            difference_type;
    typedef value_type*                          pointer;
    typedef value_type                          reference;

    const_key_iterator(const_dict_iterator curr, const_dict_iterator end);
    const_key_iterator operator++();
    const_key_iterator operator++(int);
    bool operator==(const_key_iterator &other);
    bool operator!=(const_key_iterator &other);
    std::string operator*() const;

  private:
    const_dict_iterator m_curr, m_end;
  };

  const_key_iterator keys_begin() const;
  const_key_iterator keys_end() const;

  typedef Vexpp_node_iterator<Vexpp_node_iterator_type>       iterator;
  typedef Vexpp_node_iterator<Vexpp_node_iterator_const_type> const_iterator;

  // Operator[] for dictionaries and lists
  size_t size();
  iterator operator[](const std::string &key);
  iterator operator[](size_t i);
  const_iterator operator[](const std::string &key) const;
  const_iterator operator[](size_t i) const;

  // Convertors
  int to_int() const;
  int to_int_amount(const std::string &unit) const;
  double to_double() const;
  double to_double_amount(const std::string &unit) const;
  char to_char() const;
  Date to_date() const;
  std::string to_string() const;
  const char *to_c_string() const;


  // operators for ARRAY and DICT
  iterator begin();
  iterator end();
  const_iterator begin()const ;
  const_iterator end() const;

  // operators for DICT
  iterator begin(const std::string &name);
  iterator end(const std::string &name);
  const_iterator begin(const std::string &name) const;
  const_iterator end(const std::string &name) const;
  // for python
  iterator begin_key(const char *name);
  iterator end_key(const char *name);


  void set_key(const std::string &key, Self& value);
  void append(const Self& value);
  void join(Self& value);

  // for ARRAY and DICT
  size_t size() const;

  // type
  Type get_type();
  const char *type();

  std::ostream &print(std::ostream &out, int indent) const;
private:
  Type        m_type;
  std::string name;
  Array       lst;
  Dict        dict;
};

// IO operators
std::ostream &operator<<(std::ostream &out, const Vexpp_node& data);

#include <vex/Vexpp_iterator.h>

#endif // VEXPP_NODE_H
