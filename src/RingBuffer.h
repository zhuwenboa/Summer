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
    //实际可使用空间
    static const size_t KInitialSize = 1024;

    explicit Buffer(size_t initialSize = KInitialSize)
    : buffer_(initialSize),
      readIndex_(0),
      writeIndex_(0),
      capacity_(initialSize)
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
    {
        if(writeIndex_ >= readIndex_)
            return writeIndex_ - readIndex_;
        return capacity_ - readIndex_ + writeIndex_;
    }

    //可以写入的字节数
    size_t writeableBytes() const 
    {
        if(writeIndex_ >= readIndex_)
           return capacity_ - writeIndex_ + readIndex_;
        return readIndex_ - writeIndex_;
    }

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
    {
        if(writeIndex_ + len >= capacity_ )
        {
            writeIndex_ = writeIndex_ + len - capacity_;
        }
        else 
            writeIndex_ += len;
    }

    const char* findCrlf() const 
    {
       return NULL;                
    }

    //返回buffer中的所有数据
    std::string retireAllthing() 
    {
        std::string result;
        if(writeIndex_ < readIndex_)
        {
            result = std::string(peek(), capacity_-readIndex_ -1);
            result += std::string(begin(), writeIndex_);
        }
        else 
        {
            result = std::string(peek(), writeIndex_);  
        }
        retrieveAll();
        return result;
    }

    //将缓冲区的数据写出，更新readIndex的位置
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if(len < readableBytes())
        {
            if(writeIndex_ < readIndex_)
            {
                readIndex_ = (readIndex_ + len) % capacity_;
            }
            else 
                readIndex_ += len;
        }
        else 
            retrieveAll();
    }
    //清空缓冲区
    void retrieveAll()
    {
        readIndex_ = 0;
        writeIndex_ = 0;
    }
    //将数据追加进缓冲区
    void append(const char* data, size_t len)
    {
        makeSpace(len);
        std::copy(data, data+len, beginWrite());
        modifyWirte(len);
    }
    void writeInto(std::vector<char> s)
    {
        if(writeIndex_ >= readIndex_)
        {
            if(writeIndex_ + s.size() >= capacity_)
            {
                int after_space = capacity_ - writeIndex_;
                std::copy(s.begin(), s.begin() + after_space, beginWrite());
                std::copy(s.begin()+after_space, s.end(), buffer_.begin());
                writeIndex_ = s.size() - after_space + 1;
                return;
            }
        }
        std::copy(s.begin(), s.end(), beginWrite());
    }

    ssize_t readFd(int fd)
    {
        char extrabuf[65536];
        struct iovec vec[2];
        const size_t writeable = writeableBytes();
        std::vector<char> tmp(writeable);
        vec[0].iov_base = &*tmp.begin();
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
            writeInto(tmp);
        }
        //不够容纳则将extrabuf上收到的数据追加到buffer中
        else 
        {
            writeInto(tmp);
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
        //可以写入区域小于len，则需扩容
        if (writeableBytes() < len)
        {
            std::vector<char> tmp_buf(capacity_+len);
            std::string tmp = retireAllthing();
            for(auto a : tmp)
                tmp_buf.emplace_back(a);
            buffer_ = std::move(tmp_buf);
            readIndex_ = 0;
            writeIndex_ = tmp.size();
            capacity_ = tmp_buf.size();
        }
    }


private:  
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;   
    size_t capacity_;

    static const char Crlf[]; 
};


}//namespace Summer


#endif