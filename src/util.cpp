#include "util.h"
#include<string.h>
#include<errno.h>
#include<time.h>
#include<sys/time.h>
#include<stdio.h>
#include<sstream>
#include<thread>

namespace Summer
{
    thread_local char errnobuf_[128];
    thread_local char tid_buf_[32];

    //线程安全的，因为使用的是thread_local变量
    const char* strerr(int erro)
    {
        strerror_r(erro, errnobuf_, sizeof(errnobuf_));
        return errnobuf_;
    }
    const char* strerrno()
    {
        return strerr(errno);
    }

    const char* getThreadIdStr()
    {
        std::ostringstream os;
        os << std::this_thread::get_id();
        strncpy(tid_buf_, os.str().c_str(), sizeof(tid_buf_));
        return tid_buf_;
    }

    int getNowtime()
    {
        struct timeval now;
        gettimeofday(&now, nullptr);
        int timenow = static_cast<int>(now.tv_sec);
        return timenow;
    }

}