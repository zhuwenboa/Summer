#ifndef SOCKOPS_H
#define SOCKOPS_H

#include<arpa/inet.h>

namespace Summer{
namespace sockets{
int createNonblockingOrdie(sa_family_t family);

int connect(int sockfd, struct sockaddr* addr);
void bindOrDie(int sockfd, struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec* iov, int iovcount);
ssize_t write(int sockfd, const void* buf, size_t count);
void close(int sockfd);
void shutdownwrite(int sockfd);

void toIpPort(char* buf, size_t size, struct sockaddr* addr);
void toIp(char* buf, size_t size, struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

int getSocketError(int sockfd);

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);

}//namespace sockets;

} //namespace Summer
#endif