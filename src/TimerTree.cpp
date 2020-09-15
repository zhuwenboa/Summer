#include"Timer.h"
#include"TimerTree.h"
#include"Eventloop.h"
#include<sys/timerfd.h>
#include<functional>
#include<iostream>
#include "util.h"
using namespace Summer;

int create_timerfd()
{
    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    assert(timerfd != -1);
    return timerfd;
}
/*
int Summer::getNowtime()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    int timenow = static_cast<int>(now.tv_sec);
    return timenow;
}
*/

TimerTree::TimerTree(Eventloop* loop)
    : loop_(loop),
      timerId_(1),
      timerfd_(create_timerfd()),
      timerfdChannel_(loop_, timerfd_)
{
    //将timerfd对应的channel加入到IO多路复用中
    timerfdChannel_.setReadCallback(std::bind(&TimerTree::handleRead, this));
    //注册可读事件
    timerfdChannel_.enableReading();
}

TimerTree::~TimerTree()
{
    timerfdChannel_.unableAll();
    timerfdChannel_.reMove();
    ::close(timerfd_);
}

size_t TimerTree::addTimer(Timer* ti)
{
    size_t id = timerId_++;
    //确保线程安全
    loop_->runInLoop(std::bind(&TimerTree::addTimerInloop, this, ti, id));
    return id;
}

void TimerTree::addTimerInloop(Timer* ti, size_t id)
{
    ti->setId(id);
    auto it = std::shared_ptr<Timer>(ti);
    timer_.insert(it);
    idMap_.insert({id, it});
    std::cout << "定时事件成功添加\n";
}

void TimerTree::cancelTimer(size_t id)
{
    loop_->runInLoop(std::bind(&TimerTree::cancelTimerInloop, this, id));
}

void TimerTree::cancelTimerInloop(size_t id)
{
    auto it = idMap_.find(id);
    if(it != idMap_.end())
    {
        /*
        因为multiset存在重复的元素，所以我们需要equal_range找到相同元素的起点和终点
        equal_range返回pair， first代表起始迭代器，sencond为终点迭代器 [begin, end]
        然后比较其对应的id，因为id是独一无二的
        */
        auto range = timer_.equal_range(it->second);
        for(auto a = range.first; a != range.second; ++a)
        {
            if( (*a)->id() == id)
            {
                (*a)->canceled();
                timer_.erase(a);
                break;
            }
        }
    }
}

void TimerTree::updateTimer(int now)
{
    auto tm = *timer_.begin();
    if(tm->interval() > 0)
    {
        Timer* new_tm = new Timer(now + tm->interval(), tm->interval(), tm->getTimecb());
        new_tm->setId(tm->id());
        timer_.erase(tm);
        addTimerInloop(new_tm, new_tm->id());
    }
    else 
        timer_.erase(tm);
}



void TimerTree::start()
{
    struct itimerspec new_value;
    new_value.it_value.tv_sec = 5;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 10;
    new_value.it_interval.tv_nsec = 0;

    //启动timerfd定时器
    int ret = timerfd_settime(timerfd_, 0, &new_value, nullptr);
    assert(ret != -1);
}

void TimerTree::handleRead()
{
    uint64_t one;
    int ret = ::read(timerfd_, &one, sizeof(one));
    assert(ret != -1);
    tick();
}

void TimerTree::tick()
{
    /*
    为什么不能用for循环，因为在处理完定时函数后要进行删除，删除之后迭代器会失效
    所以不能用for(auto it = timer_.begin(); it != timer_.end(); ++it){}
    只能用while每次获取timer.begin()
    */
    int now = getNowtime();
    while(!timer_.empty())
    {
        auto it = *timer_.begin();
        if(it->expire() > now)
            break;
        it->timecb();
        //如果没有要删除定时器，则将其加上当前时间继续加入到定时器中
        if(!it->isCancel())
            updateTimer(now);
        else 
        {
            idMap_.erase(it->id());
            timer_.erase(it);
        }
    }
}