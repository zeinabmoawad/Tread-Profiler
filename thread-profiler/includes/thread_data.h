#include <chrono>
#include <iomanip> 
#include <pthread.h>
#include <string>
using namespace std;
#ifndef THREADMETRICS_H
#define THREADMETRICS_H
struct ThreadMetrics
{   
    // Data of thread
    pthread_t threadId;
    string threadName;
    std::chrono::duration<double> startTime;
    std::chrono::duration<double> endTime;
    std::chrono::duration<double> waitingTime;
    int mutexAcquired;
    string state;
    // Last Mutex which thread waiting and having
    pthread_mutex_t *waitingForMutex; 
    pthread_mutex_t* havingMutex;
    // Change data to json to send
    std::string toJSON() const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "{"
            << "\"threadName\": \"" << threadName << "\", "
            << "\"threadId\": \"" << threadId << "\", "
            << "\"startTime\": \"" << startTime.count() << "\", "
            << "\"endTime\": \"" << endTime.count() << "\", "
            << "\"waitingTime\": " << waitingTime.count() << ", "
            << "\"mutexAcquired\": " << mutexAcquired << ", "
            << "\"state\": \"" << state << "\""
            << "}";
        return oss.str();
    }
};

#endif