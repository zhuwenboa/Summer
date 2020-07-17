#ifndef IOMULTIPLEXINGBASE_H
#define IOMULTIPLEXINGBASE_H

#include<map>
#include<vector>
#include "noncopyable.h"
#include "Eventloop.h"

namespace Summer{

class Channel;
class Eventloop;

//该基类为了epoll poll 和select都能继承
class IomultiplexingBase : noncopyable
{
public: 
/*
    enum Choose
    {
        POLL = 0,
        EPOLL,
        SELECT
    };
*/
    IomultiplexingBase(Eventloop* loop);
    virtual ~IomultiplexingBase();

    static IomultiplexingBase* GetNewIomultiplexing(unsigned int choice, Eventloop* loop);
    //纯虚函数
    //等待IO事件
    virtual void wait(std::vector<Channel*>* channelList, int timeout) = 0; 
    //更改channel
    virtual void modChannel(Channel* a_channel) = 0;
    //删除channel
    virtual void rmChannel(Channel* a_channel) = 0;

    virtual bool hasChannel(Channel* a_channel) const;
    
    void assertInLoopThread() const 
    {
        loops_->assertInLoopThread();
    }
protected:  
    std::map<int, Channel*> channelMap;

private: 
    //根据基类的选择，返回不同的对象
    Eventloop* loops_;    

};

}
#endif