#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include"noncopyable.h"
#include<atomic>
#include<mutex>
#include<sys/syscall.h>
#include<unistd.h>
#include<functional>
#include<memory>
#include<vector>
#include"TimerTree.h"
#include"Callback.h"

namespace Summer
{
class Channel;
class Epoll;

class Eventloop : public noncopyable
{
public:  
    Eventloop();
    ~Eventloop();

    typedef std::function<void()> Functor;
    //每个IO线程只能有一个Eventloop类
    void loop();

    void quit();

    int64_t iteration() const {return iteration_;}

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    size_t queueSize() const;


    void wakeUp();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread()
    {
        if(!isInLoopThread()) 
        {
            abortNotInLoopThread();            
        }       
    }
    //获得当前线程运行ID
    pid_t Gettid() const
    {
        return syscall(SYS_gettid);
    }
    bool isInLoopThread() const 
    {
        return threadId_ == Gettid();
    }

    bool eventHandling() const {return eventHandling_;}

    static Eventloop* getEventLoopOfCurrentThread();

    //定时器相关函数
    size_t runAfter(int timeout, const TimerCallback cb);
    size_t runEvery(int interval_time, const TimerCallback cb);
    void cancelTime(size_t id);
    

private:  
    void abortNotInLoopThread();
    void handleRead(); //wake up
    void doPendingFunctors();

    bool onloop_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callPendingFunctors_;
    int64_t iteration_; //Eventloop::loop()循环的次数
    const pid_t threadId_; //线程ID

    //由eventloop管理其生命周期
    std::unique_ptr<Epoll> Epoll_loop;
    std::unique_ptr<TimerTree> timerTree_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<Channel*> activeChannel;   //活动的事件集
    Channel* currentChannel;
    
    //pendingFunctors可以被其它线程访问，所以需要锁来保证线程安全
    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};



} //namespace Summer


#endif