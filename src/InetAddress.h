#ifndef INETADDRESS_H
#define INETADDRESS_H

#include<netinet/in.h>
#include<string>
#include"SockOps.h"

namespace Summer{


class InetAddress
{
public:
    //禁止隐式转换
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
    InetAddress (const char* IP, uint16_t port);
    explicit InetAddress (const struct sockaddr_in& addr) : addr_(addr) {}

    struct sockaddr* getSockAddr() {return sockets::sockaddr_cast(&addr_);}
    void setSockAddr(struct sockaddr_in& addr) {addr_ = addr;}

    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;    

private:   
    struct sockaddr_in addr_;
};

} //namespace Summer;


#endif