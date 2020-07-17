#include<iostream>
#include<sys/epoll.h>
#include<unistd.h>
#include "Epoll.h"
#include<assert.h>
#include<string.h>
#include"Channel.h"
using namespace Summer;

namespace
{
    const int Knew = -1;
    const int KAdded = 1;
    const int KDeleted = 2;
}

Epoll::Epoll(Eventloop* loop) 
    : IomultiplexingBase(loop),
    epollfd(epoll_create1(EPOLL_CLOEXEC)),
    events_(InitEventsSize)
{
    if(epollfd < 0)
    {

    }
}

Epoll::~Epoll()
{
    close(epollfd);
}

void Epoll::wait(std::vector<Channel*>* channelList, int timeout)
{
    int num_Events = epoll_wait(epollfd, 
                                &*events_.begin(), 
                                static_cast<int>(events_.size()),
                                timeout);
    int savedErrno = errno;
    
    if(num_Events > 0)
    {
        fillChanel(channelList, num_Events);
        //如果事件集数量等于events_的大小，扩容两倍
        if(num_Events == static_cast<int>(events_.size()))
            events_.resize(events_.size()* 2);
    }
    //在timeout时间内没有事件集返回
    else if(num_Events == 0)
    {

    }
    //epoll_wait发生错误
    else
    {
        
    }
    
    
}

void Epoll::fillChanel(std::vector<Channel*>* activeChannels, int nums)
{
    for(int i = 0; i < nums; ++i)
    {
        //从event中提取出channel指针
        Channel* channel_ = static_cast<Channel*>(events_[i].data.ptr);
        int fd = channel_->Fd();
        //从map中找到该描述符对应的channle
        auto it = channelMap.find(fd);
        assert(it != channelMap.end());
        assert(channel_ == it->second);

        channel_->set_Callrevent(events_[i].events);
        activeChannels->push_back(channel_);
    }   
}


//封装epoll_ctl_add/mod，内部调用update函数
void Epoll::modChannel(Channel* a_channel)
{
    //IomultiplexingBase::assertInLoopThread();
    const int index = a_channel->index();
    if(index == Knew || index == KDeleted)
    {
        int fd = a_channel->Fd();
        if(index == Knew)
        {
            assert(channelMap.find(fd) == channelMap.end());
            channelMap[fd] = a_channel;
        }
        else
        {
            assert(channelMap.find(fd) != channelMap.end());
            assert(channelMap[fd] == a_channel);
        }
        a_channel->set_index(KAdded);
        update(EPOLL_CTL_ADD, a_channel);
    }
    else
    {
        int fd = a_channel->Fd();
        assert(channelMap.find(fd) != channelMap.end());
        assert(channelMap[fd] == a_channel);
        assert(index == KAdded);
        if(a_channel->isNoevent())
        {
            //只是让epoll不再关注该channle上的事件，而不是删除这个事件
            update(EPOLL_CTL_DEL, a_channel);
            a_channel->set_index(KDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, a_channel);
        }        
    }
}

void Epoll::rmChannel(Channel* a_channel)
{
    int index = a_channel->index();
    int fd = a_channel->Fd();
    assert(channelMap.find(fd) != channelMap.end());
    assert(channelMap[fd] == a_channel);
    assert(index == KAdded || index == KDeleted);
    size_t n = channelMap.erase(fd);
    assert(n == 1);
    if(index == KAdded)
        update(EPOLL_CTL_DEL, a_channel);
    //set Knew
    a_channel->set_index(Knew);
}

void Epoll::update(int operation, Channel* a_channel)
{
    //初始化epollevent
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = a_channel->event();
    event.data.ptr = a_channel;
    int fd = a_channel->Fd();

    if(epoll_ctl(epollfd, operation, fd, &event) < 0)
    {
        //....

    }

}
