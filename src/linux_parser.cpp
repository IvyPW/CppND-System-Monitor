#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::cout;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T findValueByKey(std::string const& keyFilter, std::string const& filename) {
  std::string line;
  std::string key;
  T value;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    T val;
    while (linestream >> key >> val) {
      if (key == keyFilter) {
        value = val;
        break;
      };
    }
  }
  stream.close();
  return value;
}

template <typename T>
T getValueOfFile(std::string const& filename) {
  std::string line;
  T value;
  std::ifstream stream(kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  stream.close();
  return value;
};

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return string();
}

string LinuxParser::Kernel() {
  string os;
  string kernel;
  string version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  float per = 0.0;
  float mem_free = findValueByKey<float>("MemFree:", kMeminfoFilename);
  float mem_total = findValueByKey<float>("MemTotal:", kMeminfoFilename);
  try {
    per = (mem_total - mem_free) / mem_total;
  } catch (...) {
    per = 0;
  }
  return per;
}

long LinuxParser::UpTime() {
  long value = getValueOfFile<long>(kUptimeFilename);
  return value;
}

long LinuxParser::Jiffies() {
  auto jiffies = CpuUtilization();
  long total = 0;
  for (auto i : jiffies) {
    total += stol(i);
  }
  return total;
}

long LinuxParser::ActiveJiffies(int pid) {
  vector<string> pjiffies;
  string line;
  string cpu;
  string value;
  long pactive = 0;
  std::ifstream filestream(LinuxParser::kProcDirectory + std::to_string(pid) +
                           LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      pjiffies.push_back(value);
    }
    pactive = stol(pjiffies[13]) + stol(pjiffies[14]) + stol(pjiffies[15]) +
              stol(pjiffies[16]);
  }
  return pactive;
}

long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  long active =
      stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
      stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) +
      stol(jiffies[CPUStates::kSoftIRQ_]) + stol(jiffies[CPUStates::kSteal_]);
  return active;
}

long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();
  long idle =
      stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);

  return idle;
}

vector<string> LinuxParser::CpuUtilization() {
  vector<string> jiffies;
  string line;
  string cpu;
  string value;
  std::ifstream filestream(LinuxParser::kProcDirectory +
                           LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      jiffies.push_back(value);
    }
  }
  return jiffies;
}

int LinuxParser::TotalProcesses() {
  int value = findValueByKey<int>("processes", kStatFilename);
  return value;
}

int LinuxParser::RunningProcesses() {
  int value = findValueByKey<int>("procs_running", kStatFilename);
  return value;
}

string LinuxParser::Command(int pid) {
  return std::string(
      getValueOfFile<std::string>(std::to_string(pid) + kCmdlineFilename));
}

string LinuxParser::Ram(int pid) {
  const std::string kPRamFile = std::to_string(pid) + "/" + kStatusFilename;
  // Using VmRSS instead of VmSize to get the exact physcial mem usage
  int value = findValueByKey<int>("VmRSS:", kPRamFile);
  value = value / 1024;
  return to_string(value);
}

string LinuxParser::Uid(int pid) {
  string line;
  std::string kPStatusFile = std::to_string(pid) + "/" + kStatusFilename;
  string value = findValueByKey<string>("Uid:", kPStatusFile);
  return value;
}

string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string line;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      string user, passwd, userid;
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> passwd >> userid) {
        if (uid == userid) {
          return user;
        }
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string line;
  std::string s_pid = std::to_string(pid);
  std::ifstream filestream(kProcDirectory + s_pid + "/" + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      string value;
      std::istringstream linestream(line);
      int index = 0;
      while (linestream >> value) {
        if (index == 21) {
          long starttime = stol(value) / sysconf(_SC_CLK_TCK);
          long sysuptime = UpTime();
          return sysuptime - starttime;
        } else {
          index++;
        }
      }
    }
  }
  return 0;
}
