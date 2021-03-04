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
    Eventloop* baseloop_; //主线程
    bool start_;
    int sumThread;
    int next_;
    std::vector<std::unique_ptr<EventloopThread>> threads_;
    std::vector<Eventloop*> loops_; //保存的是栈上的对象所以不需要用智能指针管理
};


} //namespace Summer


#endif