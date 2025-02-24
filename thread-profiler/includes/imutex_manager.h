#include<pthread.h>
#ifndef IMUTEXMANAGER_H
#define IMUTEXMANAGER_H
// Mutex Interface 
class IMutexManager{
    public:
    virtual pthread_mutex_t createMutex() =0;
    virtual void acquireMutex(pthread_mutex_t &mutex)=0;
    virtual void releaseMutex(pthread_mutex_t &mutex)=0;
    virtual void destroyMutex(pthread_mutex_t &mutex) =0;
    virtual ~IMutexManager()=default;
};
#endif