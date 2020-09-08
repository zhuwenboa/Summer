#include "Eventloop.h"
#include"Channel.h"
#include"Epoll.h"
#include<pthread.h>
#include<iostream>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/eventfd.h>
#include<assert.h>
#include<algorithm>
#include"Timer.h"
#include"TimerTree.h"
using namespace Summer;

__thread Eventloop* t_loopInThisThread = 0; //线程局部存储

/*
wakeupfd 可以用eventfd或者socketpair都可以.只要是全双工通信的
*/

extern int Summer::getNowtime();

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(evtfd < 0)
    {
        //打印日志
        abort();   
    }   
    return evtfd;     
}

const int WaitTimes = 10000;

class IgnoreSigPipe
{
public:  
    IgnoreSigPipe()
    {
        //忽略SIGPIPE信号
        signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe sigobj; 

Eventloop* Eventloop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

Eventloop::Eventloop()
    : onloop_(false),
      quit_(false),
      eventHandling_(false),
      callPendingFunctors_(false),
      iteration_(0),
      threadId_(Gettid()),
      Epoll_loop(new Epoll(this)),
      timerTree_(new TimerTree(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentChannel(NULL)
{
    //判断线程有没有eventloop对象，一个线程只能有一个对象
    if(t_loopInThisThread)
    {
        //打印日志
    }
    else
    {
        t_loopInThisThread = this;
    }

    //注册ReadCallback函数
    wakeupChannel_->setReadCallback(std::bind(&Eventloop::handleRead, this));
    //注册可读
    wakeupChannel_->enableReading();
}

Eventloop::~Eventloop()
{
    wakeupChannel_->unableAll();
    wakeupChannel_->reMove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void Eventloop::loop()
{
    std::cout << Gettid() << " loop函数运行\n";
    timerTree_->start();
    assert(!onloop_);
    //检查该线程是否有Eventloop对象
    assertInLoopThread();
    onloop_ = true;
    quit_ = false;

    while(!quit_)
    {
        //先将activeChannels清空
        activeChannel.clear();
        //调用Epoll::wait()函数获取最新的channel
        Epoll_loop->wait(&activeChannel, WaitTimes);
        ++iteration_;
        
        eventHandling_ = true;
        //处理最新的channle
        for(Channel* channel : activeChannel)
        {
            currentChannel = channel;
            //调用Channle::handleEvent()处理
            currentChannel->handleEvent();
        }
        currentChannel = NULL;
        eventHandling_ = false;
        doPendingFunctors();
    }
}

void Eventloop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeUp();
    }
}

//将回调函数做成线程安全的并且无需用锁
void Eventloop::runInLoop(Functor cb)
{
    //如果用户调用函数的线程和Eventloop在同一个线程，回调就立刻执行
    if(isInLoopThread())
    {
        cb();
    }
    //否则就将其加入到队列中，IO线程会被唤醒来调用这个Functor
    else
    {
        queueInLoop(std::move(cb));   
    }
    
}

void Eventloop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    /*
    如果不是当前IO线程调用queueInloop或者阻塞在epoll_wait则需要唤醒
    */
    if(!isInLoopThread() || callPendingFunctors_)
    {
        wakeUp(); //wakeUp()函数不存在线程安全问题，所以可以直接调用
    }
}

size_t Eventloop::queueSize() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    return pendingFunctors_.size();
}

void Eventloop::updateChannel(Channel* channel)
{
    //判断channel中的eventloop和当前IO线程的eventloop是否相等
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    //Epoll::modChannel
    Epoll_loop->modChannel(channel);
}

void Eventloop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    //如果正在处理event事件集，则需要检查要删除的channel是否在activeChannle容器中
    if(eventHandling_)
    {
        assert( currentChannel == channel || 
                std::find(activeChannel.begin(), activeChannel.end(), channel) == activeChannel.end() );
    }
    Epoll_loop->rmChannel(channel);
}

bool Eventloop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return Epoll_loop->hasChannel(channel);
}

void Eventloop::abortNotInLoopThread()
{
    //打印日志
}

//唤醒自己
void Eventloop::wakeUp()
{
    //eventfd发送的数据必须为8字节
    uint64_t one = 1;
    //此处先用write函数
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        //
    }
}

//接收线程唤醒
void Eventloop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        //   
    }
}

/*
* 该函数不是简单地在临界区内依次调用Functor，而是把回调列表swap()到局部变量functors中，这样一方面减小了临界区的长度
*(意味着不会阻塞其它线程调用queueInLoop())，另一方面也避免了死锁(因为Functor可能再调用queueInLoop)。
*/
void Eventloop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        functors.swap(pendingFunctors_);
    }
    //将别的线程调用本Eventloop对象的函数放在其自己的线程中执行（防止产生竞争）
    for(const Functor& functor : functors)
    {
        functor();
    }
    callPendingFunctors_ = false;
}

//定时类函数
size_t Eventloop::runAfter(int timeout, const TimerCallback cb)
{
    int expire = getNowtime() + timeout;
    Timer* t = new Timer(expire, 0, std::move(cb));
    size_t id = timerTree_->addTimer(t);
    return id;
}

size_t Eventloop::runEvery(int interval_time, const TimerCallback cb)
{
    int expire = getNowtime() + interval_time;
    Timer* t = new Timer(expire, interval_time, std::move(cb));
    size_t id = timerTree_->addTimer(t);
    return id;
}

void Eventloop::cancelTime(size_t id)
{
    timerTree_->cancelTimer(id);
}




