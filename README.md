# ProfilerUsingChromeTracingTool

This project was about creating a profiler lib (as simple as possible) to be able to inspect the code.<br>
The result was a pretty configurable single header file that can outputs json compatible with "chrome://tracing" & the newer https://ui.perfetto.dev/.<br>

It's Thread-Safe and easily toggleable using #DEFINE ENABLE_PROFILING before importing.<br>

## Features
<ul>
  <li>Enable or Disable profiling just by using a preprocessor definition</li>
  <li>0 impact when disabled</li>
  <li>Select what function to annotate</li>
  <li>Select what thread to annotate</li>
  <li>Choose when to stop profiling</li>
  <li>Function names automatically inserted</li>
</ul>

## Multi-Thread example

### Code Overview

```c++
#define ENABLE_PROFILING
#include "profiling.hpp"

#include <thread>
#include <random>

static thread_local std::mt19937 gen1;
static thread_local std::mt19937_64 gen2;

// ====================================================================

void test1() {
    PROFILE_FUNC_START();

    unsigned int v = 0;
    for (int i = 0; i < 1000000; ++i)
        v += gen1();

    PROFILE_FUNC_END();
}

void test2() {
    PROFILE_FUNC_START();

    unsigned long long v = 0;
    for (int i = 0; i < 1000000; ++i)
        v += gen2();

    PROFILE_FUNC_END();
}

void thread_test(int id) {
    PROFILE_THREAD_INIT(id);

    if (id == 1)
        for (int i = 0; i < 10; ++i)
            test1();

    if (id == 2)
        for (int i = 0; i < 10; ++i)
            test2();

    if (id == 3)
        // Exit

    PROFILE_THREAD_TERM();
}

// ====================================================================

int main() {
    PROFILE_INIT("./test.json");
    auto t1 = std::thread(thread_test, 1);
    auto t2 = std::thread(thread_test, 2);
    auto t3 = std::thread(thread_test, 3);
    t1.join();
    t2.join();
    t3.join();
    PROFILE_TERM();
    return 0;
}

// ====================================================================
```

### Output json
<img src="https://github.com/user-attachments/assets/7bc64dc0-f3e7-4eec-894f-5509b3a12676" width="200px" />

### Preview - UIPerfetto
![image](https://github.com/user-attachments/assets/9fcbc09e-1b95-4989-8314-0a0ff19383bf)

