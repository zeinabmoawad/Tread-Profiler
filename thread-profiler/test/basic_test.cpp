#include <iostream>
#include <unistd.h>
#include "thread_profiler.h"
using namespace std;
using namespace chrono;

void *workerFunction(void *arg)
{
    ThreadProfiler *profiler = static_cast<ThreadProfiler *>(arg);
    pthread_t threadId = profiler->profilerGetThreadId();
    // Simulate work by sleeping for a while
    usleep(10000 * 1000);
    // Simulate completing some work
    profiler->profilerThreadEnd(threadId);
    return nullptr;
}

int main()
{
    ThreadProfiler profiler;
    // Start threads
    for (int i = 0; i < 100; i++)
    {
        profiler.profilerThreadStart(workerFunction, &profiler, "Thread" + to_string(i));
    }
    // Wait for threads to complete
    const auto &infoMap = profiler.profilerGetAllThread();
    for (const auto &thread_id : infoMap)
    {
        profiler.profilerThreadJoin(thread_id);
    }
    return 0;
}