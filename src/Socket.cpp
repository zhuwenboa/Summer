#include"Socket.h"
#include "InetAddress.h"
#include "SockOps.h"

#include<netinet/in.h>
#include<netinet/tcp.h>
#include<stdio.h>
#include<string.h>
using namespace Summer;

Socket::~Socket()
{
    //关闭连接
    sockets::close(sockfd_);
}

void Socket::bindAddress(InetAddress& localaddr)
{
    sockets::bindOrDie(sockfd_, localaddr.getSockAddr());
}

//监听套接字
void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if(connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }   
    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReuserPort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on)
    {
        //...
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = 1;
    if(on)
    {
        int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, 
                               static_cast<socklen_t>(sizeof(optval)));
    }
}

