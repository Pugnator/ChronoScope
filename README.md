## Description:

ChronoScope is a straightforward, header-only C++ profiler for easy performance tracking in any C++ project,  
offering insights into function call frequency and execution times to help pinpoint performance issues.

## Features:
    
- [x] Function Call Recording: Automatically logs the number of calls to each function, helping pinpoint hotspots.  
- [x] Time consumption: Captures the time spent in functions with microsecond accuracy using modern C++11 <chrono> features.  
- [x] Simplified Integration: Easily integrates into existing projects with minimal setup, thanks to macro-based instrumentation.  
- [x] Thread-Safety: Utilizes mutexes to ensure thread-safe operation, making it suitable for multi-threaded applications.  
- [x] Cross-Platform Compatibility: Compatible with various compilers and platforms, including MSVC, GCC, and Clang.  

## Getting Started:

To use ChronoScope in your project, include the header file and use the RECORD_CALL() macro to begin profiling.

## Example Usage:

```cpp
#include "chronoscope.h"

void myFunction() {
  RECORD_CALL();
  // Function implementation...
}

int main() {
  // Begin profiling
  myFunction();
  // ... more code ...
  
  // End profiling and generate report
  Profiler::getInstance().dumpReport("report.txt");
}
```
