#include"Timer.h"

using namespace Summer;

Timer::Timer(int expire, int interval, const TimerCallback cb)
    : expire_(expire),
      interval_(interval),
      timecb_(cb)
{

}

Timer::~Timer()
{

}

bool Timer::operator < (const Timer& tmp) const
{
    return expire_ < tmp.expire();
}