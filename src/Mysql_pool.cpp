#include "Mysql_pool.h"
#include<assert.h>

using namespace Summer;

std::shared_ptr<Mysql_pool> Mysql_pool::connectPool = NULL;

Mysql_pool::Mysql_pool()
{
    m_CurConn = 0;
    m_FreeConn = 0;
}

Mysql_pool::~Mysql_pool()
{
    DestroyPool();
    if(connectPool.get() != NULL)
        connectPool.reset();
}

std::shared_ptr<Mysql_pool> Mysql_pool::GetInstance()
{
    if(connectPool.get() == NULL)
    {
        connectPool = std::make_shared<Mysql_pool>();
    }
    return connectPool;
}

//构造初始化
void Mysql_pool::init(std::string url, std::string user, std::string pwd, 
              std::string dataBasename, int Port, int Maxconn)
{
    m_url = url;
    m_User = user;
    m_Password = pwd;
    m_DatabaseName = dataBasename;
    m_Port = Port;

    for(int i = 0; i < Maxconn; ++i)
    {
        MYSQL* con = NULL;
        con = mysql_init(con);
        assert(con != NULL);
        con = mysql_real_connect(con, url.c_str(), m_User.c_str(), m_Password.c_str(),
                                 m_DatabaseName.c_str(), m_Port, NULL, 0);
        connList.push_back(con);
        ++m_FreeConn;
    }  
    m_MaxConn = m_FreeConn;
}

MYSQL* Mysql_pool::GetConnection()
{
    MYSQL* con = NULL;
    if(connList.size() == 0)
        return NULL;
    std::unique_lock<std::mutex> lk(mutex_);
    while(m_FreeConn == 0)
    {
        cond_.wait(lk);
    }
    con = connList.front();
    connList.pop_front();
    --m_FreeConn;
    ++m_CurConn;
    lk.unlock();
    return con;
}

//释放当前所使用的连接
bool Mysql_pool::ReleaseConn(MYSQL* conn)
{
    if(conn == NULL)
        return false;
    mutex_.lock();
    connList.push_back(conn);
    ++m_FreeConn;
    --m_CurConn;
    mutex_.unlock();
    cond_.notify_one();
    return true;   
}

//销毁数据库连接池
void Mysql_pool::DestroyPool()
{
    mutex_.lock();
    if(connList.size() > 0)
    {
        std::list<MYSQL*>::iterator it;
        for(it = connList.begin(); it != connList.end(); ++it)
        {
            MYSQL* con = *it;
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;
        connList.clear();
    }
    mutex_.unlock();
}

//获取当前空闲的连接数
int Mysql_pool::GetFreeConn()
{
    std::lock_guard<std::mutex> lk(mutex_);
    return m_FreeConn;
}
