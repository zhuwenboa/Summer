#include "TcpConnection.h"
#include "Channel.h"
#include "Eventloop.h"
#include"SockOps.h"
#include<errno.h>
#include<iostream>
using namespace Summer;

TcpConnection::TcpConnection(Eventloop* loop, int sockfd, int id,InetAddress& localAddr, InetAddress& peerAddr)
    : loop_(loop),
    state_(Connecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    id_(id)
{
    //注册Channel类的回调函数
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErroCallback(std::bind(&TcpConnection::handleError, this));
    
    //开启心跳机制
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == Connecting);
    setState(Connected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
    //加入定时器
    timeId_ = loop_->runAfter(30, std::bind(&TcpConnection::handleTimeout, this));
}

void TcpConnection::handleRead()
{
    loop_->assertInLoopThread();
    ssize_t n = inputBuffer_.readFd(socket_->fd());
    if(n > 0)
    {
        //更新定时器
        updateTime();
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    //关闭连接
    else if(n == 0)
    {
        handleClose();
    }
    else 
    {
        handleError();
    }
}

//事件可写
void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
        ssize_t n = sockets::write(channel_->Fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if(n > 0)
        {
            //更新写缓冲区中的readIndex的位置
            outputBuffer_.retrieve(n);
            //将缓冲区中的所有数据写完
            if(outputBuffer_.readableBytes() == 0)
            {
                //停止关注可写事件
                channel_->unableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                //确保数据都发送完成在关闭连接
                if(state_ == Disconnecting)
                {
                    shutdownInloop();
                }
            }
            //updateTime();
        }
        else 
        {
            //error
        }
    }
}

void TcpConnection::handleClose()
{
    std::cout << "连接断开\n";
    loop_->assertInLoopThread();
    setState(Disconnected);
    //清除所有channel关注的事件
    channel_->unableAll();
    //删除对应的定时器
    loop_->cancelTime(timeId_);
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    //...
    std::cout << __LINE__ << "TcpConnection::handleError is running\n";
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if(state_ == Connected)
    {
        setState(Disconnected);
        channel_->unableAll();
    }  
    //将channel事件删除
    channel_->reMove();  
}

void TcpConnection::send(const char* message, size_t len)
{
    if(state_ == Connected)
    {
        if(loop_->isInLoopThread())
        {
            sendInloop(message, len);
        }
        else    
        {
            //定义一个函数指针, 不能用std::bind, 因为send有重载函数，其不知道要用哪个函数。
            void (TcpConnection::*func)(const char* message, size_t len) = &TcpConnection::send;
            loop_->runInLoop(std::bind(func, this, message, len));
        }
    }
}

void TcpConnection::send(const std::string& message, size_t len)
{
    send(message.data(), len);
}

void TcpConnection::send(const void* message, int len)
{
    send(reinterpret_cast<const char*>(message), static_cast<size_t>(len));
}

void TcpConnection::sendInloop(const char* data, size_t len)
{
    loop_->assertInLoopThread();
    //已经写入的字节数
    ssize_t nwrite = 0;
    //剩余未写入的字节数
    size_t remaining = len;

    bool writeErro = false;

    //channel没有关注写事件或者写缓冲区没有待发送的数据，则将本次数据直接写入
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrite = sockets::write(channel_->Fd(), data, len);
        updateTime();
        if(nwrite > 0)
        {
            remaining = len - nwrite;
            //数据一次写完
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else 
        {
            nwrite = 0;
            if(errno != EWOULDBLOCK)
            {
                //...
                //如果连接已经被关闭或者未连接成功
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    writeErro = true;
                }
            }
        }
    }

    //数据一次没有写完或者之前还有未发送完的数据
    if(!writeErro && remaining > 0)
    {
        //缓冲区之前剩余数据的长度
        size_t oldlen = outputBuffer_.readableBytes();
        //将未写完的数据加入到写缓冲区中
        outputBuffer_.append(data + nwrite, remaining);
        //如果没有关注可写事件则关注
        if(!channel_->isWriting())
            channel_->isWriting();
    }
}

//在设置时间内该客户没有给服务器发送任何消息，则需要试探其是否还存在
void TcpConnection::handleTimeout()
{
    std::cout << "定时函数运行\n";
    //发送一个心跳包
    std::string beat_ = "##alive?";
    int len = 9;
    if(loop_->isInLoopThread())
    {
        len = sockets::write(socket_->fd(), beat_.data(), len);
        if(len < 0)
        {
            handleClose();
            return;
        }
    }
    updateTime();
    //send(beat_, len);
}

void TcpConnection::updateTime(int newTime)
{
    //删除之前的定时器
    loop_->cancelTime(timeId_);
    timeId_ = loop_->runAfter(newTime, std::bind(&TcpConnection::handleTimeout, this));
}

//主动发起关闭
void TcpConnection::shutdown()
{
    if(state_ == Connected)
    {
        setState(Disconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInloop, this));
    }   
}

//半关闭确保可以将对方数据正确读取完整
void TcpConnection::shutdownInloop()
{
    loop_->assertInLoopThread();
    //是否关注可写事件
    if(!channel_->isWriting())
    {
        //关闭写端
        socket_->shutdownWrite();
    }
}