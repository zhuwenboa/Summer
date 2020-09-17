#include"Connector.h"
#include"Channel.h"
#include"Eventloop.h"
#include"SockOps.h"

#include<errno.h>

using namespace Summer;

Connector::Connector(Eventloop* loop, const InetAddress& addr)
        : loop_(loop),
          C_addr_(addr),
          connect_(false),
          st_(cDisConnected),
          retryDelayMs_(cInitRetryDelayMs)
{

}

Connector::~Connector()
{

}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInloop, this));
}

void Connector::startInloop()
{
    if(connect_)
    {
        connect();
    }
    else 
    {
        //打印日志
    }
}

void Connector::stop()
{
    connect_  = false;
    loop_->queueInLoop(std::bind(&Connector::stopInloop, this));
}

void Connector::stopInloop()
{
    if(st_ == cConnecting)
    {
        SetState(cDisConnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

//非阻塞connect
void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrdie(C_addr_.family());
    int ret = sockets::connect(sockfd, C_addr_.getSockAddr());
    //判断是否连接成功
    int saveErro = (ret == 0) ? 0 : errno;
    switch (saveErro)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:  
        case EISCONN: //连接已经建立成功
            connecting(sockfd);
            break;
        //出现以下错误进行重试
        case EAGAIN:  
        case EADDRINUSE:
        case EADDRNOTAVAIL:  
        case ECONNREFUSED:  
        case ENETUNREACH:  
            retry(sockfd);
            break;
        case EACCES:  
        case EPERM:  
        case EAFNOSUPPORT:  
        case EALREADY:  
        case EBADF:  
        case EFAULT:  
        case ENOTSOCK:  
            sockets::close(sockfd);
            break; 
        default:  
            sockets::close(sockfd);
            break;
    }
}

//重新开始
void Connector::restart()
{
    loop_->assertInLoopThread();
    SetState(cDisConnected);
    retryDelayMs_ = cInitRetryDelayMs;
    connect_ = true;
    startInloop();
}

void Connector::connecting(int sockfd)
{
    SetState(cConnecting);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErroCallback(std::bind(&Connector::handleErro, this));
    channel_->enableWriting(); //关注可写事件
}

int Connector::removeAndResetChannel()
{
    channel_->unableAll();
    channel_->reMove();
    int sockfd = channel_->Fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    //如果状态为连接进行中
    if(st_ == cConnecting)
    {
        int sockfd = removeAndResetChannel();
        //判断sockfd是连接成功还是出错
        //因为出错套接字也是返回可读可写
        int err = sockets::getSocketError(sockfd);
        //连接失败则重试
        if(err)
        {
            retry(sockfd);
        }
        //连接成功
        else
        {
            //设置当前connector的状态
            SetState(cConnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else 
                sockets::close(sockfd);
        }
    }
    else 
    {
        //未知的可写请求
    }
}

void Connector::handleErro()
{
    //如果还在进行连接，将其有关资源删除并进行重试
    if(st_ == cConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    //先将之前的关闭
    sockets::close(sockfd);
    SetState(cDisConnected);
    //过一段时间进行连接重试
    if(connect_)
    {
        loop_->runAfter(retryDelayMs_ / 1000, std::bind(&Connector::startInloop, shared_from_this()) );
        //增加重试时间
        retryDelayMs_ = std::min(retryDelayMs_ * 2, cMaxRetryDelayMs);
    }
}

