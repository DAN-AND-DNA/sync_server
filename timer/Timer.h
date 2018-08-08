#pragma once

#include <memory>
#include <queue>

namespace dan
{
class SyncServer;


namespace eventloop
{
class Channel;
class RedisChannel;
class EventLoop;
}

namespace timer
{

class Timer : public std::enable_shared_from_this<Timer>
{
public:
    Timer();

    Timer(uint64_t ulExpireTime, std::string&& strOpenid, dan::eventloop::EventLoop* pstEventLoop, std::shared_ptr<dan::SyncServer>& pstServer, bool bIsUser = true);

    ~Timer();

    void Init();

    int Fd(){return m_iFd_;}

    bool IsInit(){return m_bIsInit_;}

    bool IsRun(){return m_bIsRun_;}
public:
    // redis 相关业务 接口 TODO 提供move版本
    void RedisHGETALL(std::string& strOpenid);                                  
    void RedisZCARD(std::string& strKey);                                      // 数量
    void RedisZCOUNT(std::string& strKey, uint64_t iMin);                // 范围内的数量
    void RedisMULTI();
    void RedisEXEC(int iType = REDIS);
    //void RedisEXEC1();
    void RedisZRANGE(std::string& strKey, int iStart, int iStop, bool bIsTran = false);
    void RedisZREMRANGEBYRANK(std::string& strKey, int iStart, int iStop, bool bIsTran = false);     // 删除指定排名区间
    void RedisEXPIRE(std::string& strKey, int iSeconds, bool bIsTran = false);
private:    
    // 定时器触发回调
    void ClientTimeoutCallback();
    void ServerTimeoutCallback();
private:
    //static const int = 0;
    static const int REDIS = 0;
    static const int DB = 1;
    static const int RSTOREDB = 2;


    // redis 相关业务 回调
    static void RedisZCARDCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData);
    static void RedisHGETALLCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);
    static void RedisZCOUNTCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);
    static void RedisZRANGECallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);
    static void RedisToDBCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData); 
    static void RedisRestoreToDBCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);
    static void RedisResultCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);

private:
    bool                                     m_bIsRun_;

    std::string                              m_strOpenid_;

    bool                                     m_bIsInit_;

    int                                      m_iFd_;                        // timer fd

    std::unique_ptr<dan::eventloop::Channel> m_pstChannel_;                 // 和epoll交互的通道代理

    std::unique_ptr<dan::eventloop::RedisChannel> m_pstRedisChannel_;       // 和redis交互的特殊通道

    std::weak_ptr<dan::SyncServer>           m_pstServer_;                  // 所属的服务器

    bool                                     m_bIsUser_;

    static std::queue<std::string>           m_stExpiredQueue_;              // 过期队列
};


}
}
