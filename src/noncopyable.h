#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace Summer{

class noncopyable
{
public:
    noncopyable() = default;
    ~noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator = (const noncopyable&) = delete;
};
}
#endif