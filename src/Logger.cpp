#include<iostream>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<chrono>
#include"Logger.h"
#include "util.h"

using namespace Summer;

namespace Summer
{
    //开启一个专门写日志文件的线程
    Logger Back_Log_;
    thread_local char tsp_timestr_buf[32];
}

Logger::Logger()
    : quit_(false),
     fd_(-1),
     filesize_(0),
     logFlush_(FLUSH_TO_FILE),
     thread_(std::thread([this]{this->threadFunc();}))
{
    mkdir(".Log", 0777); //创建一个日志目录
}

Logger::~Logger()
{
    quit();
    thread_.join();
}

//根据时间动态生成文件名
const char* Logger::timeStr()
{
    struct tm tm;
    time_t time_ = getNowtime();
    gmtime_r(&time_, &tm);
    tm.tm_hour += 8;
    snprintf(tsp_timestr_buf, sizeof(tsp_timestr_buf),
            "%4d-%02d-%02d %02d:%02d:%02d.%03d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(time_));
    return tsp_timestr_buf;
}

void Logger::getFilename()
{
    filename_.clear();
    filename_ += "./.Log/";
    char* tmp = const_cast<char*>(timeStr());
    tmp[19] = '\0';
    filename_ += tmp;
    filename_ += ".log";
}

void Logger::creatFile()
{
    getFilename();
    fd_ = open(filename_.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
    if(fd_ < 0)
    {
        logFlush_ = FLUSH_TO_STDOUT;
        fd_ = STDOUT_FILENO;
        return;
    }
    filesize_ = 0;
}

//是否需要新创建一个日志文件
void Logger::rollFile()
{
    if(filesize_ >= log_roll_filesize)
    {
        close(fd_);
        creatFile();
    }
}

void Logger::setFlush()
{
    switch (logFlush_)
    {
        case FLUSH_TO_STDOUT:  
        {
            fd_  = STDOUT_FILENO;
            break;
        }
        case FLUSH_TO_FILE:
        {
            creatFile();
            break;
        }
        case FLUSH_TO_STDERR:   
        {
            fd_ = STDERR_FILENO;
            break;
        }
    }
}

void Logger::threadFunc()
{
    setFlush();
    while(1)
    {
        std::unique_lock<std::mutex> lk(mutex_);
        //定时唤醒，或者被信号量唤醒
        condVar_.wait(lk, std::chrono::seconds(log_flush_interval));
        if(writeBuf_.readableBytes() > 0)
            writeBuf_.swap(flushBuf_);
        if(flushBuf_.readableBytes() > 0)
            flush();
        if(quit_)
            abort();
        lk.unlock();
    }
}

void Logger::flush()
{
    while(flushBuf_.readableBytes() > 0)
    {
        ssize_t n = ::write(fd_, flushBuf_.peek(), flushBuf_.readableBytes());
        if(n < 0)
        {
            fprintf(stderr, "writeL %s ", strerrno());
        }
        filesize_ += n;
        flushBuf_.retrieve(n);
    }
    if(logFlush_ == FLUSH_TO_FILE)
        rollFile();
}

void Logger::wakeup()
{
    condVar_.notify_one();
}

void Logger::writeToBuffer(const char* s, size_t len)
{
    std::lock_guard<std::mutex> lk(mutex_);
    writeBuf_.append(s, len);
    if(writeBuf_.readableBytes() > log_buffer_max_size)
    {
        wakeup();
    }
}

void Logger::quit()
{
    Back_Log_.quit_ = true;
    Back_Log_.wakeup();
}
