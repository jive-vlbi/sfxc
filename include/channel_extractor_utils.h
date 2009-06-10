/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - SFXC/SCARIe project.
 * This file contains:
 *   - few utilitary classes and function used by
 *     some channel_extractor to build their computation
 *     tables.
 */
#ifndef CHANNEL_EXTRACTOR_H_
#define CHANNEL_EXTRACTOR_H_

class Action {
public:
  Action() {
    channel=-1;
    value=0;
    shift=0;
  }

  Action(const int&) {
    channel=-1;
    value=0;
    shift=0;
  }

  Action(int c, int v) {
    channel = c;
    value = v;
    shift = 0;
  }
  Action(const Action& a) {
    channel = a.channel;
    value = a.value;
    shift = a.shift;
  }
  int channel;
  int value;
  int shift;
private:

};

void find_add( std::vector<Action> *v, int channel, int value, int shift);
//void find_add( std::vector<Action>& v, int channel, int value, int shift);
int compute_span_out(const std::vector< std::vector<int> > &track_positions);


#endif // CHANNEL_EXTRACTOR_UTILS

