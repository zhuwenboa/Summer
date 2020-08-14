#include"Channel.h"
#include "Eventloop.h"
#include<sys/epoll.h>
#include<assert.h>
#include<vector>
using namespace Summer;

const int Channel::S_ReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::S_WriteEvent = EPOLLOUT;
const int Channel::S_NoEvent = 0;

Channel::Channel(Eventloop* loop, int fd)
    : loop_(loop),
    fd_(fd),
    events_(0),
    Call_events(0),
    index_(-1),
    logHup_(true),
    eventHandling_(false),
    addToLoop_(false)
    {}

Channel::~Channel()
{
    //析构时保证多路复用基类不再持有channel对象
    assert(!eventHandling_);
    assert(!addToLoop_);
    if(loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }    
}

void Channel::update()
{
    addToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::reMove()
{
    assert(isNoevent());
    addToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent()
{
    handleEventDispenser();
}

void Channel::handleEventDispenser()
{
    eventHandling_ = true;
    if((Call_events & EPOLLHUP) && !(Call_events & EPOLLIN))
    {
        if(logHup_)
        {
            //日志
        }
        if(closeCallback)
            closeCallback();
    }

    if(Call_events & EPOLLERR)
    {
        if(erroCallback)
            erroCallback();
    }
    if(Call_events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if(readCallback)
            readCallback(fd_, loop_);
    }
    if(Call_events & EPOLLOUT)
    {
        if(writeCallback)
            writeCallback();
    }
    eventHandling_ = false;
}

