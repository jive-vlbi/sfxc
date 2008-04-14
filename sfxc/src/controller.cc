/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "controller.h"

Controller::Controller(Node &node) : node(node) {}

Log_writer &
Controller::get_log_writer() {
  return node.get_log_writer();
}
