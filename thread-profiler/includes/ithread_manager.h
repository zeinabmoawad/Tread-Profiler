#include <pthread.h>
#include <string>
#ifndef ITHREADMANAGER_H
#define ITHREADMANAGER_H
// Thread Interface
class IThreadManager
{
public:
    virtual pthread_t startThread(string threadName, void *(*start_routine)(void *), void *arg) = 0;
    virtual void endThread(pthread_t thread_id) = 0;
    virtual void joinThread(pthread_t thread_id) = 0;
    virtual void mainThread(pthread_t thread_id) = 0;
    virtual void acquireMutexThread(pthread_t thread_id) = 0;
    virtual void releaseMutexThread(pthread_t thread_id) = 0;
    virtual ~IThreadManager() = default;
};
#endif