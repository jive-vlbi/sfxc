#include <vex/Vex++.h>

#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  assert(argc == 2);

  Vex vex(argv[1]);

  Vex::Node root = vex.get_root_node();

  std::cout << root << std::endl;

  std::cout << *root["EXPER"]["N06C2"] << std::endl;

  std::cout << *root["FREQ"]["4974.49MHz8x8MHz"] << std::endl;

  std::cout << *root["ANTENNA"]["ARECIBO"]["antenna_motion"][0] << std::endl;

  Vex::Node node = *root["ANTENNA"]["ARECIBO"];
  for (Vex::Node::iterator it = node.begin(); it != node.end(); ++it) {
    std::cout << it.key() << " " << *it << std::endl;
  }


  node = *root["FREQ"]["4974.49MHz8x8MHz"];
  for (Vex::Node::iterator it = node.begin("chan_def");
       it != node.end("chan_def"); ++it) {
    std::cout << it.key() << " " << *it << std::endl;
  }
  return 0;

}
