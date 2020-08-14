#ifndef CHANNEL_H
#define CHANNEL_H

#include "noncopyable.h"
#include<functional>
#include<memory>

namespace Summer{

class Eventloop;

class Channel : noncopyable
{
public:  
    typedef std::function<void()> EventCallback;
    typedef std::function<void(int, Eventloop*)> ReadeventCallback;

    Channel(Eventloop* loop, int fd);
    ~Channel();

    void handleEvent();

    //设置回调函数
    void setReadCallback(ReadeventCallback cb)
    {readCallback = std::move(cb);}
    void setWriteCallback(EventCallback cb)
    {writeCallback = std::move(cb);}
    void setCloseCallback(EventCallback cb)
    {closeCallback = std::move(cb);}
    void setErroCallback(EventCallback cb)
    {erroCallback = std::move(cb);}

    void tie(const std::shared_ptr<void>&);

    int Fd() const 
    {return fd_;}
    
    int event() const
    {return events_;} 

    void set_Callrevent(int ev)
    {Call_events = ev;}

    // evnets
    void enableReading()
    {
        events_ |= S_ReadEvent;
        update();
    }   
    void unableReading()
    {
        events_ &= ~S_ReadEvent;
        update();
    }
    bool isReading() {return events_ & S_ReadEvent;}
    void enableWriting()
    {
        events_ |= S_WriteEvent;
        update();   
    }
    void unableWriting()
    {
        events_ &= ~S_WriteEvent;
        update();
    }
    bool isWriting() { return events_ & S_WriteEvent;}
    void unableAll()
    {
        events_ &= S_NoEvent;
        update();
    }
    bool isNoevent() { return events_ == S_NoEvent;}    
    //---------

    //for poll/epoll
    int index() {return index_;}
    void set_index(int idx) {index_ = idx;}

    //for debug
    

    Eventloop* ownerLoop() {return loop_;}
    void reMove();
private:  

    void update();
    void handleEventDispenser();

    static const int S_NoEvent;        
    static const int S_ReadEvent;
    static const int S_WriteEvent;

    const int fd_;       //套接字描述符
    Eventloop* loop_;
    int index_;           //在poll(2)中数组的下标
    int events_;           //当前channel关注的事件集
    int Call_events;      //回调事件 用来判断执行什么回调函数
    bool logHup_;

    bool eventHandling_;
    bool addToLoop_;

    //四个回调函数
    ReadeventCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
    EventCallback erroCallback;
};


}

#endif