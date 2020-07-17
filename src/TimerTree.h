#ifndef TIMERTREE_H
#define TIMERTREE_H
#include<set>
#include<unordered_map>
#include<memory>
#include"Channel.h"

/*
这里定时器的选择为multiset(底层为红黑树)，其存取效率和时间堆效率差别不大
但是时间堆在删除定时器时不能高效的删除，而是只能标记删除，这样会造成空间的浪费。
(参加我的另一个项目 web服务器中的定时器的实现，底层就是用最小堆实现的)
所以在此我们选择红黑树作为我们定时器的底层实现。
*/

/*
Timer一般有三种驱动方法：
1: SIGALRM信号，但在多线程程序中我们一般需要避免信号的处理
2: IO多路复用的超时参数，但是超时时间设置我们不能准确的等到定时器的到达
3: Linux下特有的timerfd，也是最合适的，将其加入到IO多路复用中，当作套接字来使用
*/

namespace Summer
{
class Eventloop;
class Timer;


class TimerTree
{
public:  
    explicit TimerTree(Eventloop* loop);
    ~TimerTree();

    size_t addTimer(Timer* ti);
    int timeout();
    void cancelTimer(size_t id);
    //开启定时器
    void start();

    //心跳函数 处理所有到期事件
    void tick();
private:  
    void addTimerInloop(Timer* ti, size_t id);
    void cancelTimerInloop(size_t id);

    //获取当前时间
    int getNowtime();

    typedef std::multiset<std::shared_ptr<Timer>>::iterator Timeriterator;

    //通过迭代器删除特定的定时器
    void delTimer(const Timeriterator it) {timer_.erase(it);}
    //通过时间删除该时间所有定时器（因为multiset允许重复值出现）
    void delTimer(const int ti);
    //更新定时器
    void updateTimer(int now);
    
    void handleRead();

    Eventloop* loop_;

    std::multiset<std::shared_ptr<Timer>> timer_;
    std::unordered_map<size_t, std::shared_ptr<Timer>> idMap_;
    //定时器标识
    size_t timerId_;

    int timerfd_;
    Channel timerfdChannel_;
};

} // namespace 

#endif