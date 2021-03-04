#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include"noncopyable.h"
#include"InetAddress.h"
#include "Buffer.h"
#include"Socket.h"
#include<memory>
#include<any>
#include<functional>
#include"Callback.h"

namespace Summer
{
class Channel;
class Eventloop;
class Socket;
class TcpConnection;
//class Mysql_pool;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:  
    TcpConnection(Eventloop* loop, int sockfd, int id, InetAddress& localAddr, InetAddress& peerAddr);
    ~TcpConnection();

    Eventloop* getLoop() const
    {return loop_;}
    InetAddress localAddress() const 
    {return localAddr_;}
    InetAddress peerAddress() const
    {return peerAddr_;}
    bool connected() const 
    {return state_ == Connected;}
    bool disconnected() const 
    {return state_ == Connected;}


    void send(const char* message, size_t len);
    void send(const std::string& message, size_t len);
    void send(const void* message, int len);

    void setConnectionCallback(const ConnectionCallback& cb)
    {connectionCallback_ = cb;}

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb;}
    
    void setCloseCallback(const CloseCallback& cb)
    {closeCallback_ = cb;}

    void connectEstablished();
    void connectDestroyed();

    void shutdown();

    int fd() const 
    {return socket_->fd();}

    int getId() const 
    {return id_;}

    Buffer* inputBuffer() {return &inputBuffer_;}
    Buffer* outputBuffer() { return &outputBuffer_;}
    //更新时间
    void updateTime(int newTime = 30);

    void handleTimeout();

private:  
    enum State
    {
        Disconnected,
        Connecting, 
        Connected,
        Disconnecting
    };
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInloop(const char* data, size_t len);
    void forceCloseInloop();
    void shutdownInloop(); 
    void setState(State s) {state_ = s;}

    Eventloop* loop_;
    //const std::string name;
    State state_;
    bool reading_;
    //用unique_ptr来将Socket的生命周期和TcpConnection绑定起来
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    //std::any context_;

    ConnectionCallback connectionCallback_; 
    MessageCallback messageCallback_;//接收到消息后用户需要做什么
    WriteCompleteCallback writeCompleteCallback_; //数据发送完毕要做的函数
    CloseCallback closeCallback_;         //关闭连接时要做的

    Buffer inputBuffer_;//读缓冲区
    Buffer outputBuffer_;//写缓冲区

    int id_; //在TcpServer中connectionMap所对应的id号 
    size_t timeId_; //定时器对应的id
};
} //namespace Summer



#endif