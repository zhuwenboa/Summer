#include "EventloopThread.h"
#include"Eventloop.h"
#include<iostream>
using namespace Summer;

EventloopThread::EventloopThread()
    : loop_(nullptr),
      exiting(false),
      thread_([this]{this->threadfunc();})
{    
}

EventloopThread::~EventloopThread()
{
    exiting = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

Eventloop* EventloopThread::getLoop()
{
    std::unique_lock<std::mutex> lk(mutex_);
    while(loop_ == nullptr)
    {
        cond_.wait(lk);
    }
    return loop_;
}

void EventloopThread::threadfunc()
{
    Eventloop loop;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    std::cout << "线程 " << syscall(SYS_gettid) << " 被创建\n";
    loop.loop();
    std::lock_guard<std::mutex> lk(mutex_);
    loop_ = nullptr;    
}