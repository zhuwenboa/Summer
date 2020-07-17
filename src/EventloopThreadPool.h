#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include"noncopyable.h"
#include<functional>
#include<memory>
#include<vector>

namespace Summer
{
class Eventloop;
class EventloopThread;

class EventloopThreadPool : noncopyable
{
public:  
    EventloopThreadPool(Eventloop* baseloop);
    ~EventloopThreadPool();

    void setThreadnum(int num)
    {sumThread = num;}

    void start();

    Eventloop* getNextloop();

private:  
    Eventloop* baseloop_;
    bool start_;
    int sumThread;
    int next_;
    std::vector<std::unique_ptr<EventloopThread>> threads_;
    std::vector<Eventloop*> loops_;
};


} //namespace Summer


#endif