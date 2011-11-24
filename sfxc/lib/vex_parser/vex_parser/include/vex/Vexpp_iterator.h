#ifndef VEXPP_ITERATOR_H
#define VEXPP_ITERATOR_H

#include <cstddef>
#include <assert.h>
#include "Vexpp_node.h"



/**
 *  Vexpp_node_iterator : Iterator over a Vexpp_node
 *  the data is stored in Vexpp_node_iterator_types to have the same
 *  implementation for the const and non-const version.
 **/

template <class Vexpp_node_iterator_type>
class Vexpp_node_iterator {
public:
  typedef Vexpp_node_iterator<Vexpp_node_iterator_type>     Self;
  typedef typename Vexpp_node_iterator_type::Node           Node;
  typedef typename Vexpp_node_iterator_type::array_iterator array_iterator;
  typedef typename Vexpp_node_iterator_type::dict_iterator  dict_iterator;

  typedef typename Vexpp_node_iterator_type::Node           value_type;
  typedef std::forward_iterator_tag                         iterator_category;
  typedef ptrdiff_t                                         difference_type;
  typedef value_type*                                       pointer;
  typedef value_type&                                       reference;

  Vexpp_node_iterator();
  Vexpp_node_iterator(Node *node);
  Vexpp_node_iterator(Node *node,const array_iterator &array_it);
  Vexpp_node_iterator(Node *node,const dict_iterator &dict_it);

  bool operator==(const Self &other) const;
  bool operator!=(const Self &other) const;
  Node &operator*() const;
  Node *operator->() const;
  Self operator++();
  Self operator++(int);
  Self operator[](const std::string &key) const;
  Self operator[](size_t i);

  std::string key();
private:
  enum Type {
    NODE=0, // Reference to a node
    ARRAY,  // Reference to an item in a list
    DICT,   // Reference to an item in a dict
    EMPTY   // Default constructed item
  }
  type;
  Node           *vexp;
  array_iterator lst_it;
  dict_iterator  dict_it;
};



template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
Vexpp_node_iterator()
    : type(EMPTY), vexp(NULL), lst_it(NULL), dict_it(NULL) {}

template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
Vexpp_node_iterator(Node *vexp)
    : type(NODE), vexp(vexp), lst_it(NULL), dict_it(NULL) {}
template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
Vexpp_node_iterator(Node *vexp, const array_iterator &lst_it)
    : type(ARRAY), vexp(vexp), lst_it(lst_it), dict_it(NULL) {}
template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
Vexpp_node_iterator(Node *vexp, const dict_iterator &dict_it)
    : type(DICT), vexp(vexp), lst_it(NULL), dict_it(dict_it) {}

template <class Vexpp_node_iterator_type>
typename Vexpp_node_iterator<Vexpp_node_iterator_type>::Node &
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator*() const {
  switch (type) {
  case NODE: {
      assert(vexp != NULL);
      return *vexp;
      break;
    }
  case ARRAY: {
      assert(lst_it != Vexpp_node::array_iterator(NULL));
      assert(lst_it != vexp->lst.end());
      return *lst_it;
      break;
    }
  case DICT: {
      assert(dict_it != Vexpp_node::dict_iterator(NULL));
      assert(dict_it != vexp->dict.end());
      return dict_it->second;
      break;
    }
  case EMPTY:
  default:
    assert(false);
    return *vexp;
  }
}
template <class Vexpp_node_iterator_type>
typename Vexpp_node_iterator<Vexpp_node_iterator_type>::Node *
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator->() const {
  return &operator*();
}

template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator[](const std::string &key) const {
  // Get the node and then find the proper key
  return operator*()[key];
}

template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator[](size_t i) {
  // Get the node and then find the proper item
  return operator*()[i];
}

template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator++() {
  switch (type) {
  case ARRAY: {
      ++lst_it;
      return *this;
    }
  case DICT: {
      ++dict_it;
      return *this;
    }
  default:
    assert(false);
  }
}

template <class Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator++(int i) {
  assert(i==0);
  Vexpp_node_iterator<Vexpp_node_iterator_type> self = *this;
  switch (type) {
  case ARRAY: {
      ++lst_it;
      break;
    }
  case DICT: {
      ++dict_it;
      break;
    }
  default:
    assert(false);
  }
  return self;
}

template <class Vexpp_node_iterator_type>
bool
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator==(const Vexpp_node_iterator &other) const {
  if (type != other.type) return false;

  switch (type) {
  case EMPTY:
    return true;
  case NODE:
    return vexp == other.vexp;
  case ARRAY:
    return lst_it == other.lst_it;
  case DICT:
    return dict_it == other.dict_it;
  }
  assert(false);
  return false;
}

template <class Vexpp_node_iterator_type>
bool
Vexpp_node_iterator<Vexpp_node_iterator_type>::
operator!=(const Vexpp_node_iterator &other) const {
  return !(*this == other);
}

template <class Vexpp_node_iterator_type>
std::string
Vexpp_node_iterator<Vexpp_node_iterator_type>::
key() {
  assert(type == DICT);
  return dict_it->first;
}

struct Vexpp_node_iterator_type {
  typedef Vexpp_node                 Node;
  typedef Vexpp_node::array_iterator array_iterator;
  typedef Vexpp_node::dict_iterator  dict_iterator;
};

struct Vexpp_node_iterator_const_type {
  typedef const Vexpp_node                 Node;
  typedef Vexpp_node::const_array_iterator array_iterator;
  typedef Vexpp_node::const_dict_iterator  dict_iterator;
};

#endif // VEXPP_ITERATOR_H
