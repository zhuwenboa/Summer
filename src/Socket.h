#ifndef SOCKET_H
#define SOCKET_H
#include "noncopyable.h"
#include<netinet/tcp.h>
struct tcp_info;

namespace Summer{
class InetAddress;

//封装socket类(封装socket是为了采用RAII的方法，保证对象的生命周期)
class Socket : noncopyable
{
public:  
    //禁止隐式转换
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    int fd() const { return sockfd_;}

    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char* buf, int len) const;

    void bindAddress(InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite();

    void setTcpNoDelay(bool on);

    //设置是否可以重用addr
    void setReuseAddr(bool on);
    //设置是否可以重用port
    void setReuserPort(bool on);
    //是否开启心跳
    void setKeepAlive(bool on);
private:  
    const int sockfd_;
};


} // namespace Summer;



#endif