#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include"Buffer.h"

namespace Summer
{

class Logger;

void output(int level, const char* file, int line, const char* func, const char* format, ...);

extern int logLevel_;

//设置打印日志的级别, 只打印日志级别比logLevel_大的日志
void setlogLevel(int level);

void setlogFlush(int flush);

#define logDebug(...) \
    if(Summer::Logger::DEBUG >= Summer::logLevel_) \
        Summer::output(Summer::Logger::DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define logInfo(...) \
    if(Summer::Logger::INFO >= Summer::logLevel_) \
        Summer::output(Summer::Logger::INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define logWarn(...) \
    if(Summer::Logger::WARN >= Summer::logLevel_) \
        Summer::output(Summer::Logger::WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define logError(...) \
    if(Summer::Logger::ERROR >= Summer::logLevel_) \
        Summer::output(Summer::Logger::ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define logFatal(...) \
    if(Summer::Logger::FATAL >= Summer::logLevel_) \
        Summer::output(Summer::Logger::FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

} //namespace Summer


#endif