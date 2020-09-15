#ifndef UTIL_H
#define UTIL_H
#include<stddef.h>

namespace Summer
{
    const char* strerrno();
    const char* strerr(int err);

    const char* getThreadIdstr();

   int getNowtime();

}


#endif