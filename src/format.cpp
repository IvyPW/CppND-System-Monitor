#include "format.h"

#include <math.h>

#include <string>

using std::string;
using std::to_string;

string Format::ElapsedTime(long seconds) {
  long const sec = fmod(seconds, 60);
  seconds /= 60;
  long const min = fmod(seconds, 60);
  long const hour = seconds / 60;
  string uptime = to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);
  return uptime;
}