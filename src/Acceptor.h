//封装了接收器(listenfd)
#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include<functional>
#include"Channel.h"
#include"Socket.h"

namespace Summer
{
class Eventloop;
class InetAddress;

class Acceptor : noncopyable
{
public:  
    typedef std::function<void (int sockfd, InetAddress&)> NewConnectionCallback;

    Acceptor(Eventloop* loop, InetAddress& listenaddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {newConnectionCallback_ = cb;}

    bool listenning() const  {return listening_;}
    void listen();

private:  
    void handleRead();

    Eventloop* loop_;
    //socket是RAII封装了socket的生命周期。Acceptor的socket是listening socket，用来接收新连接。
    Socket acceptSocket_;

    //channle用于观察此socket上的readable事件，并回调Acceptor::handleRead()，后者会调用accpet来接受新连接，并回调用户callback
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
};

}//namespace Summer



#endif