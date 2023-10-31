#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include <stack>
#include <mutex>
#include <random>
#include <sstream>
#include <fstream>
#include <algorithm>

#define PROFILER_ENABLED

class Timer;

struct ProfileInfo
{
  unsigned int count = 0;
  long long duration = 0;
};

/// @brief A profiler class that records the number of calls to a function/method
/// and the time spent in a function/method
class Profiler
{
  friend class Timer;

public:
  static Profiler &getInstance()
  {
    static Profiler instance;
    return instance;
  }

  void recordTimeAndCalls(const std::string &functionName, const std::string &fileName, int lineNo, long long duration)
  {
    std::lock_guard<std::mutex> lock(mtx);
    std::stringstream ss;
    ss << fileName << ":" << lineNo << ":" << functionName;
    std::string funcIdentifier = ss.str();
    ProfileInfo &info = profileData[funcIdentifier];
    info.count++;
    info.duration += duration;
  }

  void dumpTextReport(const std::string &filename) const
  {
    if (profileData.empty())
    {      
      return;
    }

    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
      std::cerr << "Failed to open file for writing: " << filename << std::endl;
      return;
    }

    std::vector<std::pair<std::string, ProfileInfo>> entries(profileData.begin(), profileData.end());
    std::sort(entries.begin(), entries.end(),
              [](const std::pair<std::string, ProfileInfo> &a, const std::pair<std::string, ProfileInfo> &b)
              {
                if (a.second.duration != b.second.duration)
                  return a.second.duration > b.second.duration;
                return a.second.count > b.second.count;
              });

    // Write out the sorted data
    outFile << "===== Profiling Report =====\n";
    for (const auto &entry : entries)
    {
      outFile << entry.first << ": " << entry.second.duration << " us, "
              << entry.second.count << " calls\n";
    }

    outFile.close();
  }

private:
  Profiler() {}
  Profiler(Profiler const &) = delete;
  Profiler(Profiler &&) = delete;
  void operator=(Profiler const &) = delete;
  void operator=(Profiler &&) = delete;

  mutable std::mutex mtx;
  std::unordered_map<std::string, ProfileInfo> profileData;
};

/// @brief A timer class that records the time spent in a function/method
/// and reports it to the profiler
class Timer
{
public:
  Timer(const std::string &functionName, const std::string &fileName, int lineNo, Profiler &profiler)
      : funcName(functionName), fileName(fileName), lineNo(lineNo), refProfiler(profiler), start(std::chrono::high_resolution_clock::now())
  {
    std::stringstream ss;
    ss << fileName << ":" << lineNo << ":" << functionName;
    logStr = ss.str();
  }

  ~Timer()
  {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    refProfiler.recordTimeAndCalls(funcName, fileName, lineNo, duration);
  }

private:
  size_t lineNo;

  std::string logStr;
  std::string funcName;
  std::string fileName;

  Profiler &refProfiler;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#if defined(PROFILER_ENABLED)
#define RECORD_CALL() Timer timer##__LINE__(__FUNCTION__, __FILE__, __LINE__, Profiler::getInstance())
#else
#define RECORD_CALL()
#endif
