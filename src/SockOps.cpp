#include"SockOps.h"
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/uio.h>
#include<unistd.h>
#include<string.h>

using namespace Summer;

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
    //reinterpret_cast运算符是用来处理无关类型之间的转换；
    //它会产生一个新的值，这个值会有与原始参数（expression）有完全相同的比特位。
    return reinterpret_cast<sockaddr*>(addr);
}

struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr)
{
    return reinterpret_cast<sockaddr_in*>(addr);
}

int sockets::createNonblockingOrdie(sa_family_t family)
{
    //设置sockfd为非阻塞
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
    {
        //...
    }
    return sockfd;
}
//绑定端口
void sockets::bindOrDie(int sockfd, struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr_in)));
    if(ret < 0)
    {
        //...
    }
}

//监听sockfd
void sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0)
    {
        //...
    }
}

//接收连接
int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t len = sizeof(*addr);
    //accept4 可以设置connfd的属性
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0)
    {
        //....       
    }
    return connfd;
}

//发起连接
int sockets::connect(int sockfd, struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr_in)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
    if(::close(sockfd) < 0)
    {
        //...
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
    if(::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0)
    {
        //..
    }    
    return localAddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
    if(::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen) < 0)
    {
        //..
    }    
    return peerAddr;
}