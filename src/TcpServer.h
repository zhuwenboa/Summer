#ifndef TCPSERVER_H
#define TCPSERVER_H
#include"TcpConnection.h"
#include<map>

namespace Summer 
{
class Acceptor;
class Eventloop;
class EventloopThreadPool;
class Mysql_pool;

class TcpServer : noncopyable
{
public:  
    enum Option
    {
        NoreusePort,
        reusePort,
    };

    TcpServer(Eventloop* loop, InetAddress& listenAddr, Option option = NoreusePort);
    ~TcpServer();

    Eventloop* getLoop() const 
    {return loop_;}

    //设置线程池中的数目
    void SetThreadNum(int nums);

    std::shared_ptr<EventloopThreadPool> threadPool()
    {return threadPool_;}

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    {connectionCallback_ = cb;}
    
    void setMessageCallback(const MessageCallback& cb)
    {messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {writeCompleteCallback = cb;}
    
private:  
    void newConnection(int sockfd, InetAddress& peerAddr);
    void reMoveConnection(const TcpConnectionPtr& conn);
    void reMoveConnectionInloop(const TcpConnectionPtr& conn);

    typedef std::map<int, TcpConnectionPtr> ConnectionMap;

    Eventloop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventloopThreadPool> threadPool_;
    //std::shared_ptr<Mysql_pool> connPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback;

    int nexConnId_;
    ConnectionMap connections_;
};

} //namespace Summer


#endif