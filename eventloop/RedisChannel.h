#pragma once

#include <sync_server/eventloop/Channel.h>
#include <map>

struct redisAsyncContext;
struct redisContext;


namespace dan
{
//class SyncServer;
//
namespace timer
{
class Timer;
}

namespace eventloop
{
class EventLoop;


struct Msg
{
    Msg(uint32_t dwMsgID, std::string&& strOpenid, uint64_t ulExpireTime):
        m_strOpenid(std::move(strOpenid)),
        m_ulExpireTime(ulExpireTime),
        m_dwMsgID(dwMsgID)
    {}
    std::string     m_strOpenid;
    uint64_t        m_ulExpireTime;
    uint32_t        m_dwMsgID;
};

  

class RedisChannel: public dan::eventloop::Channel
{
public:
    typedef std::function<void(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData)> TCallback;

    RedisChannel(dan::eventloop::EventLoop* pstOwnerEventLoop, const char* szAddress = "127.0.0.1", int iPort = 6379, bool bIsSub = true);

    ~RedisChannel();
    
    bool IsInit();

    redisAsyncContext* RedisAsyncContext(){return m_pstRedisAsyncContext_;}

    bool IsSub(){return m_bIsSub_;}

   // void AddTimer(uint64_t ulExpireTime, int iID, std::string&& strOpenid);

public://注册去操纵 redis
   // void TodoAuth();

    //void TodoGetUser(std::string& strOpenid);           // 获取用户信息

    //void GetUserCount();

    uint32_t Seq(){return m_dwSeq_;}

    void SetSeq(uint32_t dwSeq){m_dwSeq_ = dwSeq;}

    void AddToHoldBackQueue(uint32_t dwMsgID ,std::shared_ptr<Msg>& pstMsg){m_stHoldBackQueue_[dwMsgID] = pstMsg;}

    bool IsHoldBackQueueEmpty(){return m_stHoldBackQueue_.empty();}

    uint32_t HoldBackQueueMsgIDBy(uint32_t dwMsgID){if(m_stHoldBackQueue_[dwMsgID]) return m_stHoldBackQueue_[dwMsgID]->m_dwMsgID; else return 0;}

    uint64_t HoldBackQueueExpireTimeBy(uint32_t dwMsgID){if(m_stHoldBackQueue_[dwMsgID]) return m_stHoldBackQueue_[dwMsgID]->m_ulExpireTime; else return 0;}

    std::string HoldBackQueueOpenidBy(uint32_t dwMsgID){if(m_stHoldBackQueue_[dwMsgID]) return m_stHoldBackQueue_[dwMsgID]->m_strOpenid; else return "";}

    std::shared_ptr<Msg> HoldBackQueueMsgBy(uint32_t dwMsgID) {if(m_stHoldBackQueue_[dwMsgID]) return m_stHoldBackQueue_[dwMsgID]; else return NULL;}

    void HoldBackQueueEraseBy(uint32_t dwMsgID){m_stHoldBackQueue_.erase(dwMsgID);}


private:                                        
    void RedisTodoAuth();                               // 注册鉴权
    void RedisReadCallback();                           // redis 消息到达 回调
    void RedisWriteCallback();                          // redis 消息可发 回调

    static void RedisAddRead(void* pstData);            // 注册epoll读
    static void RedisDelRead(void* pstData);            // 取消epoll读
    static void RedisAddWrite(void* pstData);           // 注册epoll写
    static void RedisDelWrite(void* pstData);           // 取消epoll写 
    static void RedisCleanup(void* pstData);            // 关闭连接后清理

    static void RedisConnectCallback(const ::redisAsyncContext* pstRedisAsyncContext, int iStatus);                     // 连接后的回调
    static void RedisDisconnectCallback(const ::redisAsyncContext* pstRedisAsyncContext, int iStatus);                  // 断开连接后的回调
    static void RedisAuthCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);           // 向redis鉴权回调
    static void RedisSubMsgCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);         // 向redis订阅回调
 //   static void RedisGetUserCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);        // 向redis订阅回调
  //  static void RedisGetUserCountCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData);   // 向redis订阅回调



private:

    ::redisAsyncContext* m_pstRedisAsyncContext_;       //
    
    ::redisContext*      m_pstRedisContext_;

    bool                 m_bIsInit_;
    
    bool                 m_bIsSub_;

    uint32_t             m_dwSeq_;                        //sub消息 ID序号

    std::map<uint32_t, std::shared_ptr<Msg>>     m_stHoldBackQueue_;              
};

}
}



