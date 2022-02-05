#include "processor.h"

#include <iostream>
#include <sstream>
#include <string>

#include "linux_parser.h"

using std::cout;
using std::string;

float Processor::Utilization() {
  long total_1, total_2, idle_1, idle_2, active_1;
  float util = 0;
  total_1 = LinuxParser::Jiffies();
  idle_1 = LinuxParser::IdleJiffies();
  active_1 = LinuxParser::ActiveJiffies();
  idle_2 = idle_;
  total_2 = total_;

  idle_ = idle_1;
  total_ = total_1;
  active_ = active_1;

  float total_delta = total_1 - total_2;
  float idle_delta = idle_1 - idle_2;
  try {
    util = (total_delta - idle_delta) / total_delta;
  } catch (...) {
    util = 0;
  }
  cout << util << std::endl;
  return util;
}