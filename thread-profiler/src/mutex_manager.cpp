#include "mutex_manager.h"
#include "thread_manager.h"

MutexManager::MutexManager()
{
}
pthread_mutex_t MutexManager::createMutex()
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return mutex;
}
void MutexManager::acquireMutex(pthread_mutex_t &mutex)
{
    pthread_mutex_lock(&mutex);
}
void MutexManager::releaseMutex(pthread_mutex_t &mutex)
{
    pthread_mutex_unlock(&mutex);
    
}
void MutexManager::destroyMutex(pthread_mutex_t &mutex)
{
    pthread_mutex_destroy(&mutex);
}
MutexManager::~MutexManager()
{
}