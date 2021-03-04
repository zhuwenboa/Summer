#include "EventloopThreadPool.h"
#include "EventloopThread.h"

#include<iostream>

using namespace Summer;

EventloopThreadPool::EventloopThreadPool(Eventloop* baseloop)
    : baseloop_(baseloop),
      start_(false),
      sumThread(0),
      next_(0)
{
}

EventloopThreadPool::~EventloopThreadPool()
{
    //不用删除线程池中的loop对象，因为它们都是栈上的对象 
}

void EventloopThreadPool::start()
{
    start_ = true;
    for(int i = 0; i < sumThread; ++i)
    {
        EventloopThread* t = new EventloopThread();
        //用智能指针管理堆上的对象
        threads_.push_back(std::unique_ptr<EventloopThread>(t));
        loops_.push_back(t->getLoop());
    }
}

Eventloop* EventloopThreadPool::getNextloop()
{
    //这一步确保如果是单线程可以正确返回loop对象。
    Eventloop* loop = baseloop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if(static_cast<size_t>(next_) >= loops_.size() )
        {
            next_ = 0;
        }
    }
    return loop;
}
