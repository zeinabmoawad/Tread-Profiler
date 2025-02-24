#include <pthread.h>
#include "imutex_manager.h"

#ifndef MUTEXMANAGER_H
#define MUTEXMANAGER_H
class MutexManager : public IMutexManager
{   
public:
    MutexManager();
    // Interface functions
    pthread_mutex_t createMutex() override;
    void acquireMutex(pthread_mutex_t &mutex) override;
    void releaseMutex(pthread_mutex_t &mutex) override;
    void destroyMutex(pthread_mutex_t &mutex) override;
    ~MutexManager();
};
#endif