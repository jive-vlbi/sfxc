#define VERBOSE

#include <iostream>

#include "channel_extractor_interface.h"
#include "channel_extractor1.h"

#include "tester.h"

#include "utils.h"
#include "timer.h"

int benchmark(Channel_extractor_interface &channel_extractor) {
}
            
void benchmark() {

}

int main() {
  Test<Channel_extractor1, Channel_extractor1>();

  benchmark();
}
