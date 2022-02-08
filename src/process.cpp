#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  ram_ = LinuxParser::Ram(pid);
  uptime_ = LinuxParser::UpTime(pid);
  user_ = LinuxParser::User(pid);
  command_ = LinuxParser::Command(pid);
}

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const {
  float util;
  float p_duration = LinuxParser::UpTime() - LinuxParser::UpTime(pid_);
  float p_active = LinuxParser::ActiveJiffies(pid_) / sysconf(_SC_CLK_TCK);
  try {
    util = p_active / p_duration;
  } catch (...) {
    util = 0.0;
  }
  return util;
}

string Process::Command() {
  string comm_display =
      (command_.size() > 50) ? command_.substr(0, 50) + "..." : command_;
  return comm_display;
}

string Process::Ram() { return ram_; }

string Process::User() { return user_; }

long int Process::UpTime() { return uptime_; }

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}