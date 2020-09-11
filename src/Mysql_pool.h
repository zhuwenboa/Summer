#ifndef MYSQL_POOL_H
#define MYSQL_POOL_H

#include<memory>
#include<list>
#include<string>
#include<mysql/mysql.h>
#include<mutex>
#include<condition_variable>

namespace Summer 
{

class Mysql_pool  
{
public:  
    MYSQL* GetConnection();    //获取数据库连接
    bool ReleaseConn(MYSQL* conn);    //释放连接
    int GetFreeConn(); //获取连接
    void DestroyPool(); //销毁所有连接

    //单例模式
    static std::shared_ptr<Mysql_pool> GetInstance();

    void init(std::string url, std::string user, std::string pwd, 
              std::string dataBasename, int Port, int Maxconn); 

    Mysql_pool();
    ~Mysql_pool();
private:  

    int m_MaxConn; //最大连接数
    int m_CurConn; //当前已使用的连接数
    int m_FreeConn; //当前空闲的连接数
    std::list<MYSQL*> connList; //连接池

    mutable std::mutex mutex_;
    std::condition_variable cond_;

private:  
    std::string m_url; //主机地址
    int m_Port; //数据库端口号
    std::string m_User; //登录数据库用户名
    std::string m_Password; //登录数据库密码
    std::string m_DatabaseName; //使用数据库名
    static std::shared_ptr<Mysql_pool> connectPool;
};

} // namespace



#endif