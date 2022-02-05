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
  string os, kernel, version;
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
  string line;
  float mem_free;
  float mem_total;
  float per = 0.0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      string key;
      float value;
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          mem_total = value;
        } else if (key == "MemAvailable") {
          mem_free = value;
        }
      }
    }
    try {
      per = (mem_total - mem_free) / mem_total;
    } catch (...) {
      per = 0;
    }
  }
  return per;
}

long LinuxParser::UpTime() {
  string line;
  string uptime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return stol(uptime);
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
  string line, cpu, value;
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
  string line, cpu, value;
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
  string line;
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return value;
        }
      }
    }
  }
  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  std::string s_pid = std::to_string(pid);
  std::ifstream filestream(kProcDirectory + s_pid + "/" + kCmdlineFilename);
  if (filestream.is_open()) {
    return string((std::istreambuf_iterator<char>(filestream)),
                  std::istreambuf_iterator<char>());
  }
  return string();
}

string LinuxParser::Ram(int pid) {
  string line;
  std::string s_pid = std::to_string(pid);
  std::ifstream filestream(kProcDirectory + s_pid + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      string key;
      int value;
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          value = value / 1024;
          return to_string(value);
        }
      }
    }
  }
  return string();
}

string LinuxParser::Uid(int pid) {
  string line;
  std::string s_pid = std::to_string(pid);
  std::ifstream filestream(kProcDirectory + s_pid + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      string key;
      string value;
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string();
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
          return starttime;
        } else {
          index++;
        }
      }
    }
  }
  return 0;
}
