#include <iostream>
#include <unistd.h>
#include "thread_profiler.h"
using namespace std;
using namespace chrono;

void *workerFunction(void *arg)
{
    ThreadProfiler *profiler = static_cast<ThreadProfiler *>(arg);

    static pthread_mutex_t mutex = profiler->profilerCreateMutex();
    pthread_t threadId = profiler->profilerGetThreadId();
    
    // Simulate starting some work
    profiler->profileMutexAcquire(threadId, mutex);
    // Simulate work by sleeping for a while
    usleep(5000 * 1000);
    // Simulate completing some work
    profiler->profileMutexRelease(threadId, mutex);
    profiler->profilerThreadEnd(threadId);
    return nullptr;
}

int main()
{
    ThreadProfiler profiler;
    // Start threads
    for (int i = 0; i < 20; i++)
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