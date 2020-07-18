#ifndef EPOLL_H
#define EPOLL_H

#include "IomultiplexingBase.h"

struct epoll_event;

namespace Summer{

class Channel;
class Eventloop;

class Epoll : public IomultiplexingBase
{
public:  
    Epoll(Eventloop* loop);
    ~Epoll() override;

    //封装epoll_wait函数
    void wait(std::vector<Channel*>* channelList, int timeout) override;
    
    //封装epoll_ctl_add/mod，内部调用update函数
    void modChannel(Channel* a_channel) override;

    //封装epoll_ctl_del
    void rmChannel(Channel* a_channel) override;


private:  
    //将活动的事件插入的channel中
    void fillChanel(std::vector<Channel*>* activeChannels, int nums);
    
    //添加/更新channel, 相当以epoll_ctl_add/mod函数
    void update(int operation, Channel* a_channel);


    int epollfd;
    static const int InitEventsSize = 32;
    std::vector<struct epoll_event> events_;
};

} //namespace

#endif