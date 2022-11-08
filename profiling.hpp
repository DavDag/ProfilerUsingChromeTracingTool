#pragma once

/**
 * Example usage:
 * 
 * int myFunc(int a, int b) {
 *    PROFILE_FUNC_START();
 *    // stuff
 *    PROFILE_FUNC_END();
 * }
 * 
 * void myThreadFun(int id, int a, int b) {
 *    PROFILE_THREAD_INIT(id);
 *    // stuff
 *    PROFILE_THREAD_TERM();
 * }
 * 
 * int main() {
 *    PROFILE_INIT("result.json");
 *    // stuff
 *    PROFILE_TERM();
 *    return 0;
 * }
 */

#ifdef ENABLE_PROFILING
    #define PROFILE_INIT(filename)        profiler::init(filename)
    #define PROFILE_TERM()                profiler::terminate()
    #define PROFILE_THREAD_INIT(threadId) profiler::thread::init((profiler::ThreadId_t) threadId)
    #define PROFILE_THREAD_TERM()         profiler::thread::terminate()
    #define PROFILE_FUNC_START()          profiler::func::start(__func__)
    #define PROFILE_FUNC_END()            profiler::func::end(__func__)
#else
    #define PROFILE_INIT(filename)
    #define PROFILE_TERM()
    #define PROFILE_THREAD_INIT(threadId)
    #define PROFILE_THREAD_TERM()
    #define PROFILE_FUNC_START()
    #define PROFILE_FUNC_END()
#endif // ENABLE_PROFILING

#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <chrono>
#include <mutex>

namespace profiler {

    using Timestamp_t = unsigned long long;
    using ThreadId_t  =   signed long long;

    // Initialize the profiler state
    void init(const char* filename);

    // Terminate the profiler state
    void terminate();

    namespace thread {

        // Initialize the profiler per-thread state
        void init(ThreadId_t threadId);

        // Terminate the profiler per-thread state
        void terminate();

    } // NAMESPACE thread

    namespace func {

        // Send a "begin" event
        void start(const char* funcname);

        // Send an "end" event
        void end(const char* funcname);

    } // NAMESPACE func

} // NAMESPACE profiler


/**
 * Implementation
 */
namespace profiler {

    namespace _internal {

        // Constants
        constexpr int MAX_EVENT_DUMP_LENGHT = 128; // Max event (as string) size

        // Global vars
        static std::mutex ioMutex;   // Critical section
        static std::fstream outFile; // Output destination
        static Timestamp_t startTs;  // Offset for each timestamp logged

        // Per-Thread attributes
        static thread_local ThreadId_t threadId;                // Thread-Id
        static thread_local char buffer[MAX_EVENT_DUMP_LENGHT]; // Internal buffer to store tmp string

        /**
         * Returns a timestamp (ns) from epoch
         */
        Timestamp_t inline timestamp() noexcept {
            const auto& now = std::chrono::steady_clock::now();
            const auto& tp = std::chrono::time_point_cast<std::chrono::microseconds>(now);
            return tp.time_since_epoch().count();
        }

        /**
         * Returns a timestamp (ns) from profiler initialization
         */
        Timestamp_t inline timestampFromInit() noexcept {
            return timestamp() - startTs;
        }

        /**
         * Do the actual log into the file.
         * 
         * To keep the critical section as small as possibile, the event is first
         * processed into a temporary buffer and only a final write into the outFile
         * is synchronized.
         */
        void inline event(const char* name, char type, Timestamp_t ts) noexcept {
            auto bytes = sprintf(
                buffer,
                "{\"name\":\"%.32s\",\"ph\":\"%c\",\"ts\":%llu,\"tid\":%llu},",
                name, type, ts, threadId
            );
            // Critical section
            {
                std::lock_guard<std::mutex> guard(ioMutex);
                outFile.write(buffer, bytes);
            }
        }

    } // NAMESPACE _internal

    void init(const char* filename) {
        // Set the starting timestamp (to offset future samples)
        _internal::startTs = _internal::timestamp();

        // Open the output file
        _internal::outFile.open(filename, std::fstream::out);

        // Write starting sequence
        _internal::outFile << "[";

        // Main thread is also a thread itself
        thread::init(0ULL);
    }

    void terminate() {
        // Main thread is also a thread itself
        thread::terminate();

        // Write ending sequence
        _internal::outFile << "]";

        // Close file
        _internal::outFile.close();
    }

    void thread::init(ThreadId_t threadId) {
        // Update internal id
        _internal::threadId = threadId;
    }

    void thread::terminate() {
        // Update internal id
        _internal::threadId = -1;
    }

    void func::start(const char* funcname) {
        // Sample timestamp
        auto ts = _internal::timestampFromInit();

        // Write event
        _internal::event(funcname, 'B', ts);
    }

    void func::end(const char* funcname) {
        // Sample timestamp
        auto ts = _internal::timestampFromInit();

        // Write event
        _internal::event(funcname, 'E', ts);
    }

} // NAMESPACE profiler
