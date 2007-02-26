#include <Controller.h>

Controller::Controller(Node &node) : node(node) {
}

Log_writer &
Controller::get_log_writer() {
  return node.get_log_writer();
}
