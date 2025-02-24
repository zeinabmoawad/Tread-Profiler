#include <pthread.h>
#include<iostream>
#include "thread_profiler.h"
#include "thread_manager.h"
#include "mutex_manager.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace std;
using boost::asio::ip::tcp;

ThreadProfiler::ThreadProfiler() : ioc(), acceptor(ioc, tcp::endpoint(tcp::v4(), 8080))
{
    endCheckDeadLock = false;
    threadManager = new ThreadManager(startThreadingTime);
    mutexManager = new MutexManager();
    threadManager->mainThread(pthread_self());
    startWebSocketServer();
}

void ThreadProfiler::startWebSocketServer()
{
    std::thread([this]()
                {
                    while (true)
                    {
                        try
                        {
                            tcp::socket socket(ioc);

                            acceptor.accept(socket);

                            ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
                            ws->accept(); // Accept WebSocket connection

                            std::cout << "WebSocket Client Connected." << std::endl;
                            sendAllDataOnUpdate();
                            checkDeadLock();
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << "WebSocket Server Error: " << e.what() << std::endl;
                        }
                    }
                })
        .detach();
}

void ThreadProfiler::sendAllDataOnUpdate()
{
    if (!ws)
        return; // If WebSocket is not connected, return

    try
    {
        std::string data;
        for (auto const &x : threadManager->getThreadInfoMap())
            data += (x.second).toJSON() + "\n";
        ws->write(boost::asio::buffer(data)); // Send data to the client
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error sending data: " << e.what() << std::endl;
    }
}

void ThreadProfiler::sendDataOnUpdate(pthread_t thread_id)
{
    if (!ws)
        return; // If WebSocket is not connected, return

    try
    {
        std::string metricsData = collectMetricsData(thread_id);
        ws->write(boost::asio::buffer(metricsData)); // Send data to the client
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error sending data: " << e.what() << std::endl;
    }
}

std::string ThreadProfiler::collectMetricsData(pthread_t thread_id)
{
    std::string data;

    data += (threadManager->getThreadInfo(thread_id)).toJSON() + "\n";
    return data;
}

void ThreadProfiler::checkDeadLock()
{
    std::thread([this]()
                {
                    while (true)
                    {
                        if (endCheckDeadLock)
                            break;
                        bool deadlock = false;
                        for (const auto &thread1 : threadManager->getThreadInfoMap())
                        {
                            pthread_mutex_t *waitingForMutex1 = thread1.second.waitingForMutex;
                            pthread_mutex_t *havingMutex1 = thread1.second.havingMutex;

                            for (const auto &thread2 : threadManager->getThreadInfoMap())
                            {
                                pthread_mutex_t *waitingForMutex2 = thread2.second.waitingForMutex;
                                pthread_mutex_t *havingMutex2 = thread2.second.havingMutex;
                                if (thread1.first != thread2.first)
                                {
                                    // Compare pointers or addresses of the mutexes instead of the objects themselves
                                    if ((waitingForMutex1 != nullptr && havingMutex2 != nullptr && waitingForMutex1 == havingMutex2) && (waitingForMutex2 != nullptr && havingMutex1 != nullptr && waitingForMutex2 == havingMutex1))
                                    {
                                        std::string deadlockMsg = "{\"type\":\"deadlock\"}\n";
                                        ws->write(boost::asio::buffer(deadlockMsg));
                                        std::cout << "Deadlock detected" << std::endl;
                                        deadlock = true;
                                        break;
                                    }
                                }
                            }
                            if (deadlock)
                                break;
                        }
                        if (deadlock)
                            break;
                    }
                })
        .detach();
}

void ThreadProfiler::closeWebSocket()
{
    if (ws && ws->is_open())
    {
        try
        {
            ws->close(websocket::close_code::normal);
            std::cout << "WebSocket connection closed gracefully." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error closing WebSocket: " << e.what() << std::endl;
        }
    }
}

void ThreadProfiler::profilerThreadStart(void *(*start_routine)(void *), void *arg, string threadName)
{
    pthread_t thread_id = threadManager->startThread(threadName, start_routine, arg);
    sendDataOnUpdate(thread_id);
}

void ThreadProfiler::profilerThreadEnd(pthread_t thread_id)
{
    threadManager->endThread(thread_id);
    sendDataOnUpdate(thread_id);
}

void ThreadProfiler::profilerThreadJoin(pthread_t thread_id)
{
    if (pthread_equal(thread_id, pthread_self()))
    {
        // Don't join the main thread to itself
        return;
    }
    threadManager->joinThread(thread_id);
}

void ThreadProfiler::profileMutexAcquire(pthread_t thread_id, pthread_mutex_t &mutex)
{
    threadManager->acquireMutexThread(thread_id);
    sendDataOnUpdate(thread_id);
    threadManager->setWaitingMutex(thread_id, mutex);

    chrono::time_point<chrono::high_resolution_clock> startWaitingTime = std::chrono::system_clock::now();
    mutexManager->acquireMutex(mutex);
    threadManager->setThreadWaitingDuration(thread_id, startWaitingTime);
    threadManager->setHavingMutex(thread_id, mutex);
    sendDataOnUpdate(thread_id);
}

void ThreadProfiler::profileMutexRelease(pthread_t thread_id, pthread_mutex_t &mutex)
{
    mutexManager->releaseMutex(mutex);
    threadManager->releaseMutexThread(thread_id);
    sendDataOnUpdate(thread_id);
}

pthread_mutex_t ThreadProfiler::profilerCreateMutex()
{
    return mutexManager->createMutex();
}

void ThreadProfiler::profilerDestroyMutex(pthread_mutex_t &mutex)
{
    mutexManager->destroyMutex(mutex);
}

pthread_t ThreadProfiler::profilerGetThreadId()
{
    return threadManager->getThreadId();
}

string ThreadProfiler:: profilerGetThreadName()
{
    pthread_t id = threadManager->getThreadId();
    ThreadMetrics info = threadManager->getThreadInfo(id);
    return info.threadName;
}

vector<pthread_t> ThreadProfiler::profilerGetAllThread()
{
    return threadManager->getAllThreadIds();
}

ThreadProfiler::~ThreadProfiler()
{
    endCheckDeadLock = true;
    profilerThreadEnd(pthread_self()); 
    closeWebSocket();                
    delete threadManager;
    delete mutexManager;
}
