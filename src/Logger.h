#ifndef LOGGER_H
#define LOGGER_H

#include<thread>
#include<mutex>
#include<condition_variable>
#include<string>
#include "noncopyable.h"
#include "Buffer.h"
#include <atomic>

namespace Summer
{

class Logger : noncopyable 
{
public:
    Logger();
    ~Logger();
    enum FLAGS 
    {
        FLUSH_TO_FILE = 1,
        FLUSH_TO_STDOUT,
        FLUSH_TO_STDERR,
    };
    enum LEVEL 
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };
    void writeToBuffer(const char *s, size_t len);
    void wakeup();
    void setFlush(int flush) { logFlush_ = flush; }

    static void quit();

    static const size_t log_buffer_max_size = 512 * 1024;
    static const size_t log_roll_filesize = 1024 * 1024 * 1024;
    static const int log_flush_interval = 1;

private:
    void flush();
    void setFlush();
    void threadFunc();
    void getFilename();
    void creatFile();
    void rollFile();
    void flush_time();

    const char *timeStr();

    Buffer writeBuf_;
    Buffer flushBuf_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condVar_;
    std::atomic_bool quit_;  //原子变量
    int fd_;
    std::string filename_;
    size_t filesize_;
    int logFlush_;
};

} //namespace Summer



#endif