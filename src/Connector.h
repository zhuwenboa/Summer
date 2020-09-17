#ifndef CONNECTOR_H
#define CONNECTOR_H
#include"noncopyable.h"
#include"InetAddress.h"
#include<memory>
#include<functional>

namespace Summer
{
class Channel;
class Eventloop;

class Connector : public std::enable_shared_from_this<Connector>, noncopyable
{
public:  
    typedef std::function<void (int sockfd)> NewConnectionCallback;

    Connector(Eventloop* loop, const InetAddress& addr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {newConnectionCallback_ = cb;}

    void start();
    void restart();
    void stop();

private:  
    //当前Connector状态
    enum Status
    {
        cDisConnected,
        cConnecting,
        cConnected
    };
    const int cMaxRetryDelayMs = 30 * 1000;
    const int cInitRetryDelayMs = 500;

    void SetState(Status s)
    {st_ = s;}
    void startInloop();
    void stopInloop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleErro();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    Eventloop* loop_;
    InetAddress C_addr_;
    bool connect_;
    Status st_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;

};

} //namespace
#endif