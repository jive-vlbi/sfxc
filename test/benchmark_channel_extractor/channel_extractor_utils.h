#ifndef CHANNEL_EXTRACTOR_H_
#define CHANNEL_EXTRACTOR_H_

class Action {
public:
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
};

void find_add( std::vector<Action> *v, int channel, int value, int shift);
void find_add( std::vector<Action>& v, int channel, int value, int shift);

#endif // CHANNEL_EXTRACTOR_UTILS
