#include "thread_manager.h"
#include <chrono>

ThreadManager ::ThreadManager(chrono::time_point<chrono::high_resolution_clock> startThreadingTime)
{
    manager_mutex = PTHREAD_MUTEX_INITIALIZER;
    this->startThreadingTime = startThreadingTime;
}

pthread_t ThreadManager::startThread(string threadName, void *(*start_routine)(void *), void *arg)
{
    pthread_mutex_lock(&manager_mutex);
    pthread_t thread_id;
    pthread_create(&thread_id, nullptr, start_routine, arg);
    ThreadMetrics info = {
        thread_id,
        threadName,
        std::chrono::system_clock::now() - startThreadingTime,
        {},
        std::chrono::duration<double>(0),
        0,
        "Running"};
    threadTracker[thread_id] = info;
    pthread_mutex_unlock(&manager_mutex);
    return thread_id;
}

void ThreadManager::endThread(pthread_t thread_id)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].endTime = std::chrono::system_clock::now() - startThreadingTime;
    threadTracker[thread_id].state = "Finished";
    pthread_mutex_unlock(&manager_mutex);
}

void ThreadManager::joinThread(pthread_t thread_id)
{
    pthread_join(thread_id, nullptr);
}

void ThreadManager::mainThread(pthread_t thread_id)
{
    pthread_mutex_lock(&manager_mutex);
    ThreadMetrics info = {
        thread_id,
        "MainThread",
        std::chrono::duration<double>(0),
        {},
        std::chrono::duration<double>(0),
        0,
        "Running"};
    threadTracker[thread_id] = info;
    pthread_mutex_unlock(&manager_mutex);
}

void ThreadManager::acquireMutexThread(pthread_t thread_id)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].mutexAcquired++;
    setThreadWaiting(thread_id);
    pthread_mutex_unlock(&manager_mutex);
}

void ThreadManager::releaseMutexThread(pthread_t thread_id)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].mutexAcquired--;
    setThreadRunning(thread_id);
    threadTracker[thread_id].havingMutex = nullptr;
    pthread_mutex_unlock(&manager_mutex);
}

void ThreadManager::setThreadWaitingDuration(pthread_t thread_id, chrono::time_point<chrono::high_resolution_clock> start_wait_time)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].waitingTime = std::chrono::system_clock::now() - start_wait_time;
    setThreadRunning(thread_id);
    pthread_mutex_unlock(&manager_mutex);
}

pthread_t ThreadManager::getThreadId()
{
    return pthread_self();
}

void ThreadManager::setState(pthread_t thread_id, string state)
{
    threadTracker[thread_id].state = state;
}

void ThreadManager::setThreadRunning(pthread_t thread_id)
{
    setState(thread_id, "Running");
};

void ThreadManager::setThreadWaiting(pthread_t thread_id)
{
    setState(thread_id, "Waiting");
};

void ThreadManager::setThreadFinished(pthread_t thread_id)
{
    setState(thread_id, "Finished");
};

void ThreadManager::setWaitingMutex(pthread_t thread_id, pthread_mutex_t &mutex)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].waitingForMutex = &mutex;
    pthread_mutex_unlock(&manager_mutex);
}

void ThreadManager::setHavingMutex(pthread_t thread_id, pthread_mutex_t &mutex)
{
    pthread_mutex_lock(&manager_mutex);
    threadTracker[thread_id].havingMutex = &mutex;
    if (threadTracker[thread_id].havingMutex == threadTracker[thread_id].waitingForMutex)
        threadTracker[thread_id].waitingForMutex = nullptr;
    pthread_mutex_unlock(&manager_mutex);
}

ThreadMetrics ThreadManager::getThreadInfo(pthread_t thread_id)
{
    return threadTracker[thread_id];
}

vector<pthread_t> ThreadManager::getAllThreadIds()
{
    vector<pthread_t> keys;
    for (auto it = threadTracker.begin(); it != threadTracker.end(); it++)
    {
        keys.push_back(it->first);
    }
    return keys;
}

map<pthread_t, ThreadMetrics> &ThreadManager::getThreadInfoMap()
{
    return threadTracker;
}

ThreadManager::~ThreadManager() {}