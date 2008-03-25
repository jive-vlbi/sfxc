/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands) * Copyright (c) 2007 University of Amsterdam (Netherlands) * All rights reserved. * * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007 *            Damien Marchal <dmarchal@science.uva.nl>, 2007 * * * This file is part of: *   - sfxc/SCARIe * This file contains: *   - few utils used by the several of the channel_extractor I *     have developped. */#include "channel_extractor.h"void find_add( std::vector<Action> *v, int channel, int value, int shift) {  for (unsigned int i=0;i<v->size();i++) {    if ( (*v)[i].channel == channel ) {      //std::cout << "Replacing: " <<  (*v)[i].value << " and => " << value << std::endl;      (*v)[i].value = (*v)[i].value | (value);      (*v)[i].shift = shift;      return;    }  }  Action a(channel,value);  v->push_back(a);
  return;
}


void find_add( std::vector<Action>& v, int channel, int value, int shift) {
  for (unsigned int i=0;i<v.size();i++) {
    if ( (v)[i].channel == channel ) {
      //std::cout << "Replacing: " <<  (*v)[i].value << " and => " << value << std::endl;
      (v)[i].value = (v)[i].value | (value);
      (v)[i].shift = shift;
      return;
    }
  }
  Action a(channel,value);
  v.push_back(a);
  return;
}



