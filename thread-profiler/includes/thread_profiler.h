#include <pthread.h>
#include "mutex_manager.h"
#include "thread_manager.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

using namespace std;
namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

#ifndef THREADPROFILER_H
#define ITHREADPROFILER_H
class ThreadProfiler
{
private:
    //Data members
    ThreadManager *threadManager;
    MutexManager *mutexManager;
    const chrono::time_point<chrono::high_resolution_clock> startThreadingTime = std::chrono::system_clock::now();
    std::shared_ptr<websocket::stream<tcp::socket>> ws;
    boost::asio::io_context ioc;
    tcp::acceptor acceptor;
    bool endCheckDeadLock;
    // Methods 
    void startWebSocketServer();
    void sendDataOnUpdate(pthread_t thread_id);
    void checkDeadLock();
    void sendAllDataOnUpdate();
    std::string collectMetricsData(pthread_t thread_id);
    void closeWebSocket();

public:
    ThreadProfiler();
    // Thread functions
    void profilerThreadStart(void *(*start_routine)(void *), void *arg, string threadName = "Thread");
    void profilerThreadEnd(pthread_t thread_id);
    void profilerThreadJoin(pthread_t thread_id);
    pthread_t profilerGetThreadId();
    string profilerGetThreadName();
    vector<pthread_t> profilerGetAllThread();
    //Mutex functions
    pthread_mutex_t profilerCreateMutex();
    void profilerDestroyMutex(pthread_mutex_t &mutex);
    void profileMutexAcquire(pthread_t thread_id, pthread_mutex_t &mutex);
    void profileMutexRelease(pthread_t thread_id, pthread_mutex_t &mutex);
    ~ThreadProfiler();
};
#endif