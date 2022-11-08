#define ENABLE_PROFILING
#include "profiling.hpp"

#include <thread>
#include <random>

static thread_local std::mt19937 gen;

void test1();
void test2();
void test3();
void thread_test(int id);

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

void test1() {
    PROFILE_FUNC_START();

    auto v = gen() % 10;
    if (v > 5) test2();
    if (v % 2 == 1) test3();
    std::this_thread::sleep_for(std::chrono::milliseconds(v));

    PROFILE_FUNC_END();
}

void test2() {
    PROFILE_FUNC_START();

    auto v = gen() % 50;
    if (v > 25) test1();
    if (v % 2 == 0) test3();
    std::this_thread::sleep_for(std::chrono::milliseconds(v));

    PROFILE_FUNC_END();
}

void test3() {
    PROFILE_FUNC_START();

    auto acc = 0;
    for (int i = 0; i < 100000; ++i) {
        auto v = gen() % 100;
        acc += v;
    }
    auto v = gen() % 100;
    if (v > 50) test3();

    PROFILE_FUNC_END();
}

void thread_test(int id) {
    PROFILE_THREAD_INIT(id);

    for (int j = 0; j < 10; ++j) {
        for (int i = 0; i < 10; ++i) test1();
        for (int i = 0; i < 10; ++i) test2();
        for (int i = 0; i < 10; ++i) test3();
    }

    PROFILE_THREAD_TERM();
}

// ====================================================================
