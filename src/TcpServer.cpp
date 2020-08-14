#include"TcpServer.h"
#include"Acceptor.h"
#include"Eventloop.h"
#include"SockOps.h"
#include"EventloopThreadPool.h"
#include<iostream>
#include<functional>

using namespace Summer;

namespace Summer
{
void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
     //默认connectioncallback，如果用户没有指定则调用它
     std::cout << "新的连接 " << conn->fd() << "连接成功\n";
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf)
{       
     //默认messageCallback函数，如果用户没有指定，我们则将收到的数据全部丢弃
     buf->retrieveAll();
}
}

TcpServer::TcpServer(Eventloop* loop, InetAddress& listenAddr, Option option)
        : loop_(loop),
          acceptor_(new Acceptor(loop, listenAddr, option == reusePort)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nexConnId_(1),
          threadPool_(new EventloopThreadPool(loop_))
{
     acceptor_->setNewConnectionCallback([this](int fd, InetAddress& peerAddr)
                                                {this->newConnection(fd, peerAddr);});
}

TcpServer::~TcpServer()
{
     loop_->assertInLoopThread();
     for(auto& item : connections_)
     {
          TcpConnectionPtr conn(item.second);
          //一定要在这一步进行reset，否则map一直持有TcpConnection对象，导致其不能正常析构，造成内存泄漏
          //reset将引用计数-1
          item.second.reset();      
          //用runInLoop是因为每个连接可能对应不同的线程，确保线程安全     
          conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
     }
}

void TcpServer::SetThreadNum(int nums)
{
     if(nums >= 0)
          threadPool_->setThreadnum(nums);
}


//能调用start函数肯定是主线程，也就是loop_所在的对象，为什么要用runInloop做成线程安全的
void TcpServer::start()
{
     //acceptor_->listen();
     threadPool_->start();
     loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
}

void TcpServer::newConnection(int sockfd, InetAddress& peerAddr)
{
     loop_->assertInLoopThread();
     Eventloop* ioLoop = threadPool_->getNextloop();
     int id = nexConnId_++;
     InetAddress localAddr(sockets::getLocalAddr(sockfd));
     TcpConnectionPtr conn(std::make_shared<TcpConnection>(ioLoop, sockfd, id, localAddr, peerAddr));
     connections_[id] = conn;
     conn->setConnectionCallback(connectionCallback_);
     conn->setMessageCallback(messageCallback_);
     conn->setWriteCompleteCallback(writeCompleteCallback);
     conn->setCloseCallback(std::bind(&TcpServer::reMoveConnection, this, std::placeholders::_1));
     //conn->setCloseCallback([this](const TcpConnectionPtr& conn){this->reMoveConnection(conn);});
     ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}
          
void TcpServer::reMoveConnection(const TcpConnectionPtr& conn)
{
     //因为TcpServer是无锁的，所以调用remove函数需要移动到TcpServer的loop中去完成
     loop_->runInLoop(std::bind(&TcpServer::reMoveConnectionInloop, this, conn));
}         

void TcpServer::reMoveConnectionInloop(const TcpConnectionPtr& conn)
{
     loop_->assertInLoopThread();    
     size_t n = connections_.erase(conn->getId());
     if(n != 1)
     {
          //...
     }
     Eventloop* ioLoop = conn->getLoop();
     //调用TcpConnection::connectDestroyed函数终止对象
     //ioLoop不属于当前线程，所以用queueInLoop来确保线程安全
     ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}