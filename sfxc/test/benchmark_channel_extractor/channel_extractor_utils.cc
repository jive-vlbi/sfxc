/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
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


