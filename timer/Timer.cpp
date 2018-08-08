#include <sys/timerfd.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <sync_server/redis/hiredis.h>
#include <sync_server/redis/async.h>

#ifdef __cplusplus 
}
#endif



#include <sys/time.h>
#include <sync_server/timer/Timer.h>
#include <sync_server/eventloop/EventLoop.h>
#include <sync_server/eventloop/Channel.h>
#include <sync_server/eventloop/RedisChannel.h>
#include <sync_server/SyncServer.h>
#include <sync_server/log/Logger.h> 

#include <string.h>

#include <unistd.h>

namespace dan
{

namespace timer
{

std::queue<std::string> Timer::m_stExpiredQueue_;


Timer::Timer(){}

Timer::Timer(uint64_t ulExpireTime, std::string&& strOpenid, dan::eventloop::EventLoop* pstEventLoop, std::shared_ptr<dan::SyncServer>& pstServer, bool bIsUser):
    m_bIsRun_(false),
    m_strOpenid_(std::move(strOpenid)),
    m_bIsInit_(true),
    m_iFd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    m_pstChannel_(new dan::eventloop::Channel(m_iFd_, pstEventLoop)),
    m_pstRedisChannel_(new dan::eventloop::RedisChannel(pstEventLoop, "127.0.0.1", 6379, false)),
    m_pstServer_(pstServer),
    m_bIsUser_(bIsUser)
    {
        if(m_iFd_ == -1)
            m_bIsInit_ = false;
        else
        {
            printf("time fd:%d\n",m_iFd_);
            struct ::itimerspec stTimerspec;
            ::bzero(&stTimerspec, sizeof stTimerspec);
            if(m_bIsUser_)
            {
                stTimerspec.it_value.tv_sec = ulExpireTime; 
            }
            else
            {
                stTimerspec.it_value.tv_sec = ulExpireTime;
                stTimerspec.it_interval.tv_sec = ulExpireTime;
            }
            if(::timerfd_settime(m_iFd_, 0, &stTimerspec, nullptr) == -1)
            {
                printf("set error:%s\n", strerror(errno));
            }
            else
            {
                if(m_bIsUser_)
                {
                    // 处理客户端 处理过期 和 间隔更新
                    m_pstChannel_->SetReadCallback(std::bind(&Timer::ClientTimeoutCallback, this));
                }
                else
                {
                    // 处理服务器 过期 和 更新 
                     m_pstChannel_->SetReadCallback(std::bind(&Timer::ServerTimeoutCallback, this));
                }

                m_pstChannel_->DisableWrite();          //FIXME  只读不写
                m_pstChannel_->EnableRead(); 



                if(!m_bIsUser_)
                {
                    // 从日志恢复//TODO 从leveldb恢复
                    std::vector<std::string> v;
                    dan::log::Logger().Restore(&v);
                
                    this->RedisMULTI();
                    for(uint64_t i = 0; i < v.size(); i++)
                    {
                        std::printf("%s\n", v[i].c_str());

                        this->RedisHGETALL(v[i]);

                        // FIXME 不知道 是否是   1)过期消息未发送(best)   2) 过期消息已发送，但回复来之前就崩了  3) 回复来了，还没处理就崩了
                    
                        // 最直接的就是假设最差就是，回复到了还没处理时就崩了(已经删了堆 ，过期中的玩家key)
                        // so: 

                        // TODO 恢复移除的堆内数据time到now
                        // TODO 恢复正在过期的  玩家信息(到不过期)
                    }

                    this->RedisEXEC(RSTOREDB);
                }
            }
        }

    }

Timer::~Timer()
{
    printf("close timer\n");
    m_pstChannel_->DisableAll();
    if(m_iFd_ > 0)
        ::close(m_iFd_);
}


void Timer::Init()
{
    if(m_pstChannel_->IsInit())
    {
        std::shared_ptr<void> pst = shared_from_this();
        m_pstChannel_->Tie(pst);
        //m_pstMod_->Tie(pst);
    }
}

void Timer::RedisHGETALL(std::string& strOpenid)
{
   if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisHGETALLCallback, this,"HGETALL %s",strOpenid.c_str()) != REDIS_OK)
   {
       printf("启动 去拿openid:%s 发生错误\n",strOpenid.c_str());
   }
}


void Timer::RedisZCARD(std::string& strKey)
{
    if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisZCARDCallback, this, "ZCARD %s", strKey.c_str()) != REDIS_OK)
    {
        printf("启动 去拿在线的的玩家数量 发生错误\n");
    }
}

void Timer::RedisZCOUNT(std::string& strKey, uint64_t iMin)
{
    if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisZCOUNTCallback, this, "ZCOUNT %s -inf %lu", strKey.c_str(), iMin) != REDIS_OK)
    {
        printf("启动 去拿在指定在线时间的的玩家数量 发生错误\n");
    }
}

void Timer::RedisMULTI()
{
    if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), NULL, NULL, "MULTI") != REDIS_OK)
    {
        printf("启动 事务失败");
    }
}

void Timer::RedisEXEC(int iType)
{
    switch(iType)
    {
        case DB:
        {
            if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisToDBCallback, this, "EXEC") != REDIS_OK)
            {
                printf("启动 执行失败");
            }
            break;
        }
        case REDIS:
        {
            if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisResultCallback, this, "EXEC") != REDIS_OK)
            {   
                printf("启动 执行失败");
            }
            break;
        }
        case RSTOREDB:
        {
            if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisRestoreToDBCallback, this, "EXEC") != REDIS_OK)
            {   
                printf("启动 执行失败");
            }
            break;

        }
        default:
        {
            printf("error: type");
        }
    }
}


/*
void Timer::RedisEXEC1()
{
    if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisEXEC1Callback, this, "EXEC") != REDIS_OK)
    {
        printf("启动 执行失败");
    }
}
*/


void Timer::RedisZRANGE(std::string& strKey, int iStart, int iStop, bool bIsTran)
{
    if(bIsTran)
    {
        if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), NULL, NULL, "ZRANGE %s %d %d", strKey.c_str(), iStart, iStop) != REDIS_OK)
        {
            printf("启动 事务去拿指定排名的玩家 发送错误\n");
        }
    }
    else
    {
         if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), RedisZRANGECallback, this, "ZRANGE %s %d %d", strKey.c_str(), iStart, iStop) != REDIS_OK)
         {
             printf("启动 去拿指定排名的玩家 发送错误\n");
         }
    }
}


void Timer::RedisZREMRANGEBYRANK(std::string& strKey, int iStart , int iStop, bool bIsTran)
{
    if(bIsTran)
    {
        if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), NULL, NULL, "ZREMRANGEBYRANK %s %d %d", strKey.c_str(), iStart, iStop) != REDIS_OK)
        {
            printf("启动 事务去删指定的排名范围内的的玩家 发送错误\n");
        }
    }
    else
    {
        //if()
    }
}


void Timer::RedisEXPIRE(std::string& strKey, int iSeconds, bool bIsTran)
{
    if(bIsTran)
    {
        if(::redisAsyncCommand(m_pstRedisChannel_->RedisAsyncContext(), NULL, NULL, "EXPIRE %s %d", strKey.c_str(), iSeconds) != REDIS_OK)
        {
            printf("启动 事务去删指定的排名范围内的的玩家 发送错误\n");
        }
    }
    else
    {
        //if()
    }
}
     


void Timer::ClientTimeoutCallback()
{
    if(auto pstServer = m_pstServer_.lock())
    {
        // 处理客户端请求
        uint64_t i = 0;
        ::read(m_iFd_, &i , sizeof(uint64_t));
        
        // TODO 添加到过期队列
        m_stExpiredQueue_.push(m_strOpenid_);
        printf("openid:%s expired\n", this->m_strOpenid_.c_str());

        
        // TODO 清理掉这个timer
        pstServer->RemoveTimer(std::move(m_strOpenid_));
        
    }
}

void Timer::ServerTimeoutCallback()
{
    
    if(auto pstServer = m_pstServer_.lock())
    {
        m_bIsRun_ = true;
        // 处理服务器自己的业务
        uint64_t i = 0;
        ::read(m_iFd_, &i , sizeof(uint64_t));
        
        printf("server expired\n");

        //::sleep(9);

        std::string strKey("testkey");
        //uint64_t ulNow = 1530279463;
        //uint64_t ulNow = 1530279463;
        
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
      //  printf("in mod:%ld\n", tv.tv_sec);
        uint64_t ulNow = tv.tv_sec;

        RedisZCOUNT(strKey, ulNow - 30);
        //RedisZCOUNT(strKey, ulNow - 7200);
    }
}


void Timer::RedisZCARDCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        printf("拿数据结果:%s\n", pstRedisReply->str);


        if(pstRedisReply->type == REDIS_REPLY_INTEGER)
        {
                //TODO  timer 的逻辑去判断  
                int lNum = static_cast<int>(pstRedisReply->integer);
                printf("count: %d \n", lNum);
        }
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     
}

void Timer::RedisHGETALLCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        printf("拿数据结果:%s\n", pstRedisReply->str);


        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                printf("%u) %s\n", i, pstRedisReply->element[i]->str);
                
            }
        }
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
}

void Timer::RedisZCOUNTCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        printf("拿数据结果:%s\n", pstRedisReply->str);


        if(pstRedisReply->type == REDIS_REPLY_INTEGER)
        {
                //TODO  timer 的逻辑去判断  
                uint32_t ulNum = static_cast<uint32_t>(pstRedisReply->integer);
                printf("in range count: %u \n", ulNum);
                if(ulNum > 0)
                {
                    Timer* pstTimer = static_cast<Timer*>( pstData);
                    pstTimer->RedisMULTI();
                    std::string strOpenid("hWX123456789");
                    std::string strKey("testkey");

                    pstTimer->RedisZRANGE(strKey, 0, 5, true);                // FIXME 上限根据实际的来(或者给算法)
                    pstTimer->RedisZREMRANGEBYRANK(strKey, 0, 5, true);

                    pstTimer->RedisEXEC(REDIS);                              // 问redis要，然后做判断
                    //FIXME 这个时候奔溃，最糟糕的无非就是堆里的信息被干掉了，不会丢玩家数据, 下次玩家重新登陆就会有堆里的数据
                }
                else
                {
                    static_cast<Timer*>( pstData)->m_bIsRun_= false;
                }
        }
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     
}


void Timer::RedisZRANGECallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_ARRAY)
                {
                    for(int j = 0; j < static_cast<int>(pstRedisReply->element[i]->elements); j++)
                    {
                         printf("%u) %s\n", j, pstRedisReply->element[i]->element[j]->str);

                    }
                    // TODO 过期 user
                }

                
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_INTEGER)
                {
                    printf("rm  %lld\n",  pstRedisReply->element[i]->integer); 
                }
                
            }
            printf("sync to db....\n");

            //FIXME 走腾讯sdk，根据返回值判断是否释放
            
            static_cast<Timer*>(pstData)->m_bIsRun_ = false;
        }
        
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        static_cast<Timer*>(pstData)->m_bIsRun_ = false;
        return;   // 直接返回,自动清理
    }
    



}

void Timer::RedisToDBCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        printf("done:EXPIRE\n");
        // 成功
        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_ARRAY)
                {
                    for(int j = 0; j < static_cast<int>(pstRedisReply->element[i]->elements); j++)
                    {
                         printf("%u) %s\n", j, pstRedisReply->element[i]->element[j]->str);

                    }
                    // TODO 给db代理发消息
                }
            }

            printf("pre:sync\n");
            
            printf("get all data , sync to db....\n");     

            //if res == true
            printf("done:sync\n");
        }

        // 空就不做
        
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     

}

void Timer::RedisRestoreToDBCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_ARRAY)
                {
                    for(int j = 0; j < static_cast<int>(pstRedisReply->element[i]->elements); j++)
                    {
                         printf("%u) %s\n", j, pstRedisReply->element[i]->element[j]->str);

                    }
                    // TODO 给db代理发消息
                }
            }

            printf("pre:sync\n");
            
            printf("get all data , sync to db....\n");     

            //if res == true
            printf("done:sync\n");
        }

        // 空就不做
        
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     

}




void Timer::RedisResultCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_ARRAY)
                {
                    Timer* pstTimer = static_cast<Timer*>(pstData);

                    //printf("todo a tran\n"); 
                    pstTimer->RedisMULTI();
                    std::string strTmp;
                    for(int j = 0; j < static_cast<int>(pstRedisReply->element[i]->elements); j++)
                    {
                         printf("%u) %s\n", j, pstRedisReply->element[i]->element[j]->str);
                         std::string strOpenid(pstRedisReply->element[i]->element[j]->str);
                         strOpenid = "hloginuser:" + strOpenid;
                         pstTimer->RedisHGETALL(strOpenid);
                         //pstTimer->RedisEXPIRE(strOpenid, 30*60);
                         pstTimer->RedisEXPIRE(strOpenid, 20, true);
                         strTmp += (strOpenid + " ");  
                    }

                    printf("pre:EXPIRE %s\n", strTmp.c_str()); 
                    
                    pstTimer->RedisEXEC(DB);     // 问redis拿玩家数据写db
                }
                
                if(pstRedisReply->element[i]->type ==  REDIS_REPLY_INTEGER)
                {
                    printf("rm  %lld\n",  pstRedisReply->element[i]->integer); 
                }
                
            }
        }
        
       
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("拿数据发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("拿数据发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     

}







}
}

