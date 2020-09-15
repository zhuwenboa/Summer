#ifndef BUFFER_H
#define BUFFER_H
#include<algorithm>
#include<vector>
#include<assert.h>
#include<string.h>
#include<sys/uio.h>
#include<string>
namespace Summer
{
//消息类型
enum Message_type
{
    MES,
    CODE,
    HEADER,
    HEART 
};

class Buffer
{
public:  
    static const size_t KReserve_space = 8;
    //实际可使用空间
    static const size_t KInitialSize = 1024;

    explicit Buffer(size_t initialSize = KInitialSize)
    : buffer_(KReserve_space + initialSize),
      readIndex_(KReserve_space),
      writeIndex_(KReserve_space)
    {
        
    }

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readIndex_, rhs.readIndex_);
        std::swap(writeIndex_, rhs.writeIndex_);
    }
    //可以读取的字节数
    size_t readableBytes() const 
    {return writeIndex_ - readIndex_;}

    //可以写入的字节数
    size_t writeableBytes() const 
    {return buffer_.size() - writeIndex_;}

    //buffer中前段剩余空间
    size_t prependableBytes() const 
    {return readIndex_;}

    const char* peek() const 
    {
        return begin() + readIndex_;
    }
    //返回可以写入的位置
    char* beginWrite()
    {return begin()+writeIndex_;}
    
    //移动writeIndex的位置
    void modifyWirte(size_t len)
    {writeIndex_ += len;}

    const char* findCrlf() const 
    {
       return NULL;                
    }

    //返回buffer中的所有数据
    std::string retireAllthing() 
    {
        std::string result(peek(), readableBytes());
        retrieveAll();
        return result;
    }

    //将缓冲区的数据写出，更新readIndex的位置
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if(len < readableBytes())
        {
            readIndex_ += len;
        }
        else 
            retrieveAll();
    }
    //清空缓冲区
    void retrieveAll()
    {
        readIndex_ = KReserve_space;
        writeIndex_ = KReserve_space;
    }
    //将数据追加进缓冲区
    void append(const char* data, size_t len)
    {
        makeSpace(len);
        std::copy(data, data+len, beginWrite());
        modifyWirte(len);
    }
    ssize_t readFd(int fd)
    {
        char extrabuf[65536];
        struct iovec vec[2];
        const size_t writeable = writeableBytes();
        vec[0].iov_base = beginWrite();
        vec[0].iov_len = writeable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof(extrabuf);

        const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;
        const ssize_t n = readv(fd, vec, 2);
        if(n < 0)
        {
            //...
        }
        //buffer可以容纳收到消息
        else if(static_cast<size_t>(n) <= writeable)
        {
            modifyWirte(n);
        }
        //不够容纳则将extrabuf上收到的数据追加到buffer中
        else 
        {
            writeIndex_ = buffer_.size();
            append(extrabuf, n - writeable);
        }
        return n;
    }
    //解析消息类型
    Message_type encodeBuf()
    {
        Message_type tp;
        //...
        switch (tp)
        {
            case MES:
                return MES;
            case CODE:  
                return CODE;
            case HEADER:  
                return HEADER;
            default:  
                return HEART;
        }
    }
private:  
    char* begin()
    {return &*buffer_.begin();}

    //重载const
    const char* begin() const
    {return &*buffer_.begin();}


    
    //移动buffer_空间
    void makeSpace(size_t len)
    {
        //可以写入区域和前面空闲区域之和小于extrabuff的长度则需要扩容
        if (writeableBytes() + prependableBytes() < len + KReserve_space)
        {
          buffer_.resize(writeIndex_+len);
        }
        //如果是不需要扩容的，则将数据的部分移动到buffer的头部
        else
        {
            assert(KReserve_space < readIndex_);
            size_t readable = readableBytes();
            std::copy(begin()+readIndex_, begin()+writeIndex_, begin()+KReserve_space);
            readIndex_ = KReserve_space;
            writeIndex_ = readIndex_ + readable;
            assert(readable == readableBytes());
        }
    }


private:  
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;   

    static const char Crlf[]; 
};


}//namespace Summer


#endif