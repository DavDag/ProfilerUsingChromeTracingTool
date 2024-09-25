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
