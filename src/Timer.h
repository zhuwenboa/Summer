#ifndef TIMER_H
#define TIMER_H
#include<unistd.h>
#include<assert.h>
#include<sys/time.h>
#include<sys/timerfd.h>
#include<functional>
#include"Callback.h"
namespace Summer
{

class Timer
{
public:  
    Timer(int expire, int interval, const TimerCallback cb);
    ~Timer();
    bool operator < (const Timer& tmp) const;
    size_t id() const {return id_;}
    int expire() const {return expire_;}
    int interval() const {return interval_;}
    void timecb() {timecb_();}
    const TimerCallback getTimecb() {return timecb_;}
    void setId(int id)
    {id_ = id;}
    void setExpire(int expire)
    {expire_ = expire;}
    void setTimecb(const TimerCallback cb)
    {timecb_ = cb;}
    bool isCancel() const {return isCancel_;}
    void canceled() {isCancel_ = true;}

private:  
    size_t id_;
    //定时器到期时间
    int expire_;
    //间隔定时器的间隔时间
    int interval_;

    TimerCallback timecb_;
    bool isCancel_;
};


} //namespace 


#endif