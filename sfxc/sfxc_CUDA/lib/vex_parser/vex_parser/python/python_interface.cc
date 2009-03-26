#include <boost/python.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>


#include "vex/Vex++.h"

using namespace boost::python;
boost::python::object
element(Vexpp_node &a, const char *arg) {
  return
    detail::make_iterator
    (boost::bind(&Vexpp_node::begin_key, _1, arg),
     boost::bind(&Vexpp_node::end_key, _1, arg),
     objects::default_iterator_call_policies(),
     detail::target(&Vexpp_node::begin_key)
    )(boost::ref(a));
}

BOOST_PYTHON_MODULE(vex_parser) {
  def("parse_vex", parse_vex);

  class_<Vexpp_node>("Vex_node")
  .def("type", &Vexpp_node::type)
  .def("to_string", &Vexpp_node::to_c_string)
  .def("__iter__", iterator<Vexpp_node>())
  .def("keys",
       range(&Vexpp_node::keys_begin,
             &Vexpp_node::keys_end))
  .def("values", &element)
  ;
}
