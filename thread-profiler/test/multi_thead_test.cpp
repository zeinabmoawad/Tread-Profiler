#include <iostream>
#include <pthread.h>
#include <unistd.h> // for usleep
#include <vector>
#include "thread_profiler.h"

using namespace std;
using namespace chrono;

void *workerFunction2(void *arg)
{
    ThreadProfiler *profiler = static_cast<ThreadProfiler *>(arg);
    pthread_t threadId = profiler->profilerGetThreadId();

    // Simulate work by sleeping
    usleep(500000); // Random sleep between 0 and 1 second

    // End profiling the thread
    profiler->profilerThreadEnd(threadId);

    return nullptr;
}
// Worker function for each thread
void *workerFunction(void *arg)
{
    ThreadProfiler *profiler = static_cast<ThreadProfiler *>(arg);

    // Create or get mutex from profiler
    static pthread_mutex_t mutex = profiler->profilerCreateMutex();
    pthread_t threadId = profiler->profilerGetThreadId();

    // Start profiling the thread
    profiler->profilerThreadStart(workerFunction2, profiler, "Worker2_Thread");

    // Acquire the mutex
    profiler->profileMutexAcquire(threadId, mutex);

    // Simulate work by sleeping
    usleep(5000 * 1000); // Random sleep between 0 and 1 second

    // Release the mutex
    profiler->profileMutexRelease(threadId, mutex);

    // End profiling the thread
    profiler->profilerThreadEnd(threadId);

    return nullptr;
}

int main()
{
    ThreadProfiler profiler;
    // Start threads
    for (int i = 0; i < 30; i++)
    {
        profiler.profilerThreadStart(workerFunction, &profiler, "Thread" + to_string(i));
    }
    // Wait for threads to complete
    const auto &infoMap = profiler.profilerGetAllThread();
    for (const auto &thread_id : infoMap)
    {       
        profiler.profilerThreadJoin(thread_id);
    }

    cout << "All threads have completed." << endl;
    return 0;
}
