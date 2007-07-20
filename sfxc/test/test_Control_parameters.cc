#include <Control_parameters.h>
#include <Log_writer_cout.h>

int main(int argc, char *argv[]) {
  char *ctrl_file = "./data/n06c2.ctrl";
  char *vex_file = "./data/n06c2.vex";
  if (argc == 3) {
    ctrl_file = argv[1];
    vex_file = argv[2];
  }
  
  Log_writer_cout log_writer;
  Control_parameters parameters;
  if (!parameters.initialise(ctrl_file, vex_file, log_writer)) {
    return 1;
  }
  
  return 0;
}
