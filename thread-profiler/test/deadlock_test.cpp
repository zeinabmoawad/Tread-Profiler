#include <iostream>
#include <pthread.h>
#include <unistd.h> // for usleep
#include <vector>
#include "thread_profiler.h"

using namespace std;
using namespace chrono;

// Worker function to simulate deadlock
void *workerFunction(void *arg)
{
    ThreadProfiler *profiler = static_cast<ThreadProfiler *>(arg);
    static pthread_mutex_t mutex1 = profiler->profilerCreateMutex();
    static pthread_mutex_t mutex2 = profiler->profilerCreateMutex();
    pthread_t threadId = profiler->profilerGetThreadId();
    int threadType = profiler-> profilerGetThreadName().find("even") != string::npos ? 0 : 1;

    if (threadType == 0)
    { // Even threads
        profiler->profileMutexAcquire(threadId, mutex1);

        // Simulate some work
        usleep(10000);

        profiler->profileMutexAcquire(threadId, mutex2);

        // Simulate work
        usleep(10000);

        // Release mutexes
        profiler->profileMutexRelease(threadId, mutex2);

        profiler->profileMutexRelease(threadId, mutex1);
    }
    else
    { // Odd threads
        profiler->profileMutexAcquire(threadId, mutex2);

        // Simulate some work
        usleep(10000);

        profiler->profileMutexAcquire(threadId, mutex1);

        // Simulate work
        usleep(10000);

        // Release mutexes
        profiler->profileMutexRelease(threadId, mutex1);

        profiler->profileMutexRelease(threadId, mutex2);
    }

    // End profiling the thread
    profiler->profilerThreadEnd(threadId);

    return nullptr;
}

int main()
{
    ThreadProfiler profiler;

    // Start threads
    for (int i = 0; i < 5; i++)
    {
        // Create even threads
        profiler.profilerThreadStart(workerFunction, &profiler, "Thread_even");

        // Create odd threads
        profiler.profilerThreadStart(workerFunction, &profiler, "Thread_odd");
    }

    const auto &infoMap = profiler.profilerGetAllThread();
    for (const auto &thread_id : infoMap)
    {
        profiler.profilerThreadJoin(thread_id);
    }

    cout << "All threads have completed." << endl;
    return 0;
}
