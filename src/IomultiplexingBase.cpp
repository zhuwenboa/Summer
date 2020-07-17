#include "IomultiplexingBase.h"
#include"Epoll.h"
#include"Channel.h"
using namespace Summer;

IomultiplexingBase::IomultiplexingBase(Eventloop* loop) : loops_(loop) {}
IomultiplexingBase::~IomultiplexingBase() = default;

bool IomultiplexingBase::hasChannel(Channel* a_channel) const 
{
    assertInLoopThread();
    //map的迭代器必须是const_iterator,因为禁止通过迭代器更改它的值，因为是排序的红黑树，更改完后可能会造成迭代器失效
    std::map<int, Channel*>::const_iterator it = channelMap.find(a_channel->Fd());
    return it != channelMap.end() && it->second == a_channel;
}
IomultiplexingBase* IomultiplexingBase::GetNewIomultiplexing(unsigned int choice, Eventloop* loop)
{
    if(choice == 1)
    {
        return new Epoll(loop);
    }
}