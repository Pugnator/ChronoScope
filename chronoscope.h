#pragma once

#if defined(__cplusplus)
#if __cplusplus < 201103L
#error "This code needs at least a C++11 compliant compiler."
#endif
#else
#error "This profiler is meant to work with a C++ compiler."
#endif

#include <unordered_map>
#include <string>
#include <iostream>
#include <stack>
#include <mutex>
#include <random>
#include <sstream>
#include <fstream>

#define SCOPE_PROFILE_CPU_CONSUMPTION 1
#define SCOPE_PROFILE_COVERAGE 2

// Select the profiling mode here by uncommenting the desired mode:
#define SCOPE_SELECTED_PROFILE SCOPE_PROFILE_CPU_CONSUMPTION
// #define SCOPE_SELECTED_PROFILE SCOPE_PROFILE_COVERAGE

#if defined(SCOPE_SELECTED_PROFILE)
#if (SCOPE_SELECTED_PROFILE != SCOPE_PROFILE_CPU_CONSUMPTION) && \
    (SCOPE_SELECTED_PROFILE != SCOPE_PROFILE_COVERAGE)
#error "Multiple profiling modes are defined. Please enable exactly one."
#endif
#else
#warning "No profiling mode selected. Release mode."
#endif

#if (SCOPE_SELECTED_PROFILE == SCOPE_PROFILE_CPU_CONSUMPTION)
#elif (SCOPE_SELECTED_PROFILE == SCOPE_PROFILE_COVERAGE)
#endif

class Timer;

/// @brief Generate a random (enough) UUID
static std::string generate_uuid()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);

  std::stringstream ss;
  ss << std::hex;

  for (int i = 0; i < 8; i++)
  {
    ss << dis(gen);
  }
  ss << "-";
  for (int i = 0; i < 4; i++)
  {
    ss << dis(gen);
  }
  ss << "-";
  for (int i = 0; i < 4; i++)
  {
    ss << dis(gen);
  }
  ss << "-";
  for (int i = 0; i < 4; i++)
  {
    ss << dis(gen);
  }
  ss << "-";
  for (int i = 0; i < 12; i++)
  {
    ss << dis(gen);
  }

  return ss.str();
}

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

  // Record a call for a given function/method name
  void recordCall(const std::string &functionName, const std::string &fileName, int lineNo)
  {
    std::lock_guard<std::mutex> lock(mtx);
    std::stringstream ss;
    ss << fileName << ":" << lineNo << ":" << functionName;
    counters[ss.str()]++;
  }

  void recordTime(const std::string &uniqueID, long long duration)
  {
    std::lock_guard<std::mutex> lock(timeMtx);
    if (timerNames.find(uniqueID) != timerNames.end())
    {
      timeCounters[timerNames[uniqueID]] += duration;
    }
  }

  void dumpReport(const std::string &filename) const
  {
    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
      std::cerr << "Failed to open file for writing: " << filename << std::endl;
      return;
    }

    try
    {      
      outFile << "===== Function Call Counts =====\n";
      if (counters.empty())
      {
        outFile << "No function calls recorded.\n";
      }
      else
      {
        for (const auto &[func, count] : counters)
        {
          outFile << func << ": " << count << " calls\n";
        }
      }

      outFile << "\n===== Time Spent (us) =====\n";
      if (timeCounters.empty())
      {
        outFile << "No timing data recorded.\n";
      }
      else
      {
        for (const auto &[func, time] : timeCounters)
        {
          if (timerNames.find(func) != timerNames.end())
          {
            outFile << timerNames.at(func) << ": " << time << " us\n";
          }
          else
          {
            outFile << func << ": " << time << " us\n";
          }
        }
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Exception occurred while writing to file: " << e.what() << std::endl;
      throw;
    }

    outFile.close();
  }

private:
  Profiler() {}
  Profiler(Profiler const &) = delete;
  Profiler(Profiler &&) = delete;
  void operator=(Profiler const &) = delete;
  void operator=(Profiler &&) = delete;

  std::unordered_map<std::string, unsigned int> counters;
  std::unordered_map<std::string, long long> timeCounters;
  std::unordered_map<std::string, std::string> timerNames;
  std::stack<std::string> timerStack;
  mutable std::mutex mtx;
  mutable std::mutex timeMtx;
  mutable std::mutex timerStackMtx;
};

/// @brief A timer class that records the time spent in a function/method
/// and reports it to the profiler
class Timer
{
public:
  Timer(const std::string &functionName, const std::string &fileName, int lineNo, Profiler &profiler)
      : refProfiler(profiler), start(std::chrono::high_resolution_clock::now())
  {
    uniqueID = generate_uuid();
    std::stringstream ss;
    ss << fileName << ":" << lineNo << ":" << functionName;
    logStr = ss.str();
    refProfiler.timerNames[uniqueID] = logStr;
  }

  ~Timer()
  {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    {
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
      refProfiler.recordTime(uniqueID, duration);
    }
  }

private:
  std::string uniqueID;
  std::string logStr;
  std::string funcName;
  std::string fileName;
  int lineNo;
  Profiler &refProfiler;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};


#if defined(SCOPE_PROFILE_COVERAGE)
#define RECORD_CALL() Profiler::getInstance().recordCall(__FUNCTION__, __FILE__, __LINE__)
#elif defined(SCOPE_PROFILE_CPU_CONSUMPTION)
#define RECORD_CALL() Timer timer##__LINE__(__FUNCTION__, __FILE__, __LINE__, Profiler::getInstance())
#else
#define RECORD_CALL()
#endif