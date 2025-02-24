#include <pthread.h>
#include <chrono>
#include <thread_data.h>
#include <map>
#include <vector>
#include <string>
#include "ithread_manager.h"
#ifndef THREADMANAGER_H
#define THREADMANAGER_H
class ThreadManager : public IThreadManager
{
private:
    map<pthread_t, ThreadMetrics> threadTracker;
    pthread_mutex_t manager_mutex;
    chrono::time_point<chrono::high_resolution_clock> startThreadingTime;

public:
    ThreadManager(chrono::time_point<chrono::high_resolution_clock> startThreadingTime);
    // Interface Functions
    pthread_t startThread(string threadName, void *(*start_routine)(void *), void *arg) override;
    void endThread(pthread_t thread_id) override;
    void joinThread(pthread_t thread_id) override;
    void mainThread(pthread_t thread_id) override;
    void acquireMutexThread(pthread_t thread_id) override;
    void releaseMutexThread(pthread_t thread_id) override;
    // getters and setters  for threadTracker info
    pthread_t getThreadId();
    vector<pthread_t> getAllThreadIds();
    ThreadMetrics getThreadInfo(pthread_t thread_id);
    map<pthread_t, ThreadMetrics> &getThreadInfoMap();
    void setState(pthread_t thread_id, string state);
    void setThreadRunning(pthread_t thread_id);
    void setThreadWaiting(pthread_t thread_id);
    void setThreadFinished(pthread_t thread_id);
    void setThreadWaitingDuration(pthread_t thread_id, chrono::time_point<chrono::high_resolution_clock> start_wait_time);
    void setWaitingMutex(pthread_t thread_id, pthread_mutex_t &mutex);
    void setHavingMutex(pthread_t thread_id, pthread_mutex_t &mutex);
    ~ThreadManager();
};

#endif