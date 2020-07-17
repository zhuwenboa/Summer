#include"Acceptor.h"
#include"Eventloop.h"
#include"InetAddress.h"
#include"SockOps.h"

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>

using namespace Summer;

Acceptor::Acceptor(Eventloop* loop, InetAddress& listenaddr, bool reuseport)
        : loop_(loop),
          acceptSocket_(sockets::createNonblockingOrdie(listenaddr.family())),
          acceptChannel_(loop, acceptSocket_.fd()),
          listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReuserPort(reuseport);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    //将所有注册的事件清空
    acceptChannel_.unableAll();
    //将acceptChannel事件删除
    acceptChannel_.reMove();
    //::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    //注册可读事件
    acceptChannel_.enableReading();
}

//接收新的连接
void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peeraddr;
    int connfd = acceptSocket_.accept(&peeraddr);
    if(connfd >= 0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
        else 
            sockets::close(connfd);
    }
    else
    {
        //...
    }
}

 