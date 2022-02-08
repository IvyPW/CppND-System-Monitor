#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() {
  cpu_ = Processor();
  return cpu_;
}

vector<Process>& System::Processes() {
  auto pids = LinuxParser::Pids();
  processes_.clear();
  for (const int i : pids) {
    Process p = Process(i);
    if (p.Command().size() > 0 && p.Ram().size() > 0) {
      processes_.emplace_back(i);
    }
  }
  return processes_;
}

std::string System::Kernel() { return LinuxParser::Kernel(); }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }