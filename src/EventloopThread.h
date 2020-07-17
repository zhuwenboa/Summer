#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include<mutex>
#include<thread>
#include<condition_variable>
#include"noncopyable.h"
#include<functional>

namespace Summer
{
class Eventloop;

class EventloopThread : noncopyable
{
public:  

    EventloopThread();
    ~EventloopThread();
    Eventloop* getLoop();
private:  
    void threadfunc();
    Eventloop* loop_;
    bool exiting;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


} //namespace Summer

#endif