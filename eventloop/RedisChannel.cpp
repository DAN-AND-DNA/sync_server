
#ifdef __cplusplus
extern "C" {
#endif

#include <sync_server/redis/hiredis.h>
#include <sync_server/redis/async.h>

#ifdef __cplusplus 
}
#endif


#include <unistd.h>
#include <sync_server/eventloop/RedisChannel.h>
#include <sync_server/SyncServer.h>
#include <functional>
#include <string.h>

namespace  dan
{
namespace eventloop
{



RedisChannel::RedisChannel(dan::eventloop::EventLoop* pstOwnerEventLoop, const char*szAddress, int iPort, bool bIsSub):
    Channel(-1, pstOwnerEventLoop, true),
    m_pstRedisAsyncContext_(::redisAsyncConnect(szAddress, iPort)),                             //1. 注册去连redis
    m_pstRedisContext_(&(m_pstRedisAsyncContext_->c)),
    m_bIsInit_(true),
    m_bIsSub_(bIsSub),
    m_dwSeq_(0)
{
    SetFd(m_pstRedisContext_->fd);

    if(m_pstRedisAsyncContext_->err || m_pstRedisAsyncContext_->ev.data)
    {
        m_bIsInit_ = false;
        printf("redis context err:%s\n", m_pstRedisAsyncContext_->errstr);
    }
    else
    {
        printf("成功分配redis fd:%d\n",Fd());

        
        m_pstRedisAsyncContext_->ev.addRead  = RedisChannel::RedisAddRead;                                    //2. 为了兼容底层网络层, 提供函数，当发送完毕请求要接受回复的时候，向epoll注册可读
        m_pstRedisAsyncContext_->ev.addWrite = RedisChannel::RedisAddWrite;                                   
        m_pstRedisAsyncContext_->ev.delRead  = RedisChannel::RedisDelRead;
        m_pstRedisAsyncContext_->ev.delWrite = RedisChannel::RedisDelWrite;
        m_pstRedisAsyncContext_->ev.cleanup  = RedisChannel::RedisCleanup;
        m_pstRedisAsyncContext_->ev.data = this;                                                


        if(m_pstRedisAsyncContext_->ev.data)
        {
            ::redisAsyncSetConnectCallback(m_pstRedisAsyncContext_, RedisConnectCallback);      //3. 设置连接回调函数
            ::redisAsyncSetDisconnectCallback(m_pstRedisAsyncContext_, RedisDisconnectCallback);

            this->SetReadCallback(std::bind(&RedisChannel::RedisReadCallback, this));           //4. 为了兼容底层网络层, 提供函数, 当redis fd可读时, 使得库可以在消息到达时去处理消息
            this->SetWriteCallback(std::bind(&RedisChannel::RedisWriteCallback, this));
            
            RedisTodoAuth();                                                                         //5. 注册去鉴权
        }
        else
        {
            printf("redis init error\n");
            m_bIsInit_ = false;
        }
    }
}

RedisChannel::~RedisChannel()
{
    if(Fd() >= 0)
        ::close(Fd());
}

bool RedisChannel::IsInit(){return ( m_bIsInit_ && dan::eventloop::Channel::IsInit() && !(m_pstRedisAsyncContext_->err));}

//void RedisChannel::AddTimer(uint64_t ulExpireTime, int iID, std::string&& strOpenid){m_pstOwnerServer_->AddTimer(ulExpireTime, iID, std::move(strOpenid));}
 
void RedisChannel::RedisTodoAuth()
{
    printf("注册鉴权\n");
    if(::redisAsyncCommand(m_pstRedisAsyncContext_, RedisAuthCallback, this, "AUTH Sh@1424Qbgame") != REDIS_OK)
    {
        printf("注册鉴权失败:%s\n", m_pstRedisAsyncContext_->errstr);
    }

}

/*
void RedisChannel::TodoGetUser(std::string& strOpenid)
{

    if(::redisAsyncCommand(m_pstRedisAsyncContext_, RedisGetUserCallback, this,"HGETALL %s",strOpenid.c_str()) != REDIS_OK)
    {
        printf("拿openid:%s 发生错误\n",strOpenid.c_str());
    }

}
*/
/*
void RedisChannel::GetUserCount()
{
    if(::redisAsyncCommand(m_pstRedisAsyncContext_, RedisGetUserCountCallback, this, "ZCARD testkey") != REDIS_OK)
    {
        printf("拿在线的的玩家数量 发生错误\n");
    }
}
*/


void RedisChannel::RedisAddRead(void* pstData)
{
    RedisChannel* pstRedisChannel = static_cast<RedisChannel*>(pstData);

    pstRedisChannel->EnableRead();
    printf("redis allow read\n");
}

void RedisChannel::RedisDelRead(void* pstData)
{
    RedisChannel* pstRedisChannel = static_cast<RedisChannel*>(pstData);

    pstRedisChannel->DisableRead();
    printf("redis dis read\n"); 
}

void RedisChannel::RedisAddWrite(void* pstData)
{
    RedisChannel* pstRedisChannel = static_cast<RedisChannel*>(pstData);
    
    pstRedisChannel->EnableWrite();
    printf("redis allow write\n");  
}

void RedisChannel::RedisDelWrite(void* pstData)
{
    RedisChannel* pstRedisChannel = static_cast<RedisChannel*>(pstData); 
    
    pstRedisChannel->DisableWrite();
    printf("redis dis write\n");
}

void RedisChannel::RedisCleanup(void*pstData)
{
    RedisChannel* pstRedisChannel = static_cast<RedisChannel*>(pstData);
   
    pstRedisChannel->DisableAll();
    printf("redis cleanup!\n");
}

void RedisChannel::RedisReadCallback()
{
    ::redisAsyncHandleRead(m_pstRedisAsyncContext_);
}

void RedisChannel::RedisWriteCallback()
{
    ::redisAsyncHandleWrite(m_pstRedisAsyncContext_);
}

void RedisChannel::RedisConnectCallback(const ::redisAsyncContext* pstContext, int iStatus)
{
    if(iStatus != REDIS_OK)
    {
        printf("连接到redis失败 error:%s\n", pstContext->errstr);
        return;
    }
     printf("连接到redis成功!\n");
/*
    printf("连接到redis成功! 开始启动鉴权\n");
    if(::redisAsyncCommand(const_cast<redisAsyncContext*>(pstContext), RedisAuthCallback, NULL, "AUTH Sh@1424Qbgame") != REDIS_OK)
    {
        printf("鉴权 启动失败:%s", pstContext->errstr);
    }
    */
}

void RedisChannel::RedisDisconnectCallback(const ::redisAsyncContext* pstContext, int iStatus)
{
    if(iStatus != REDIS_OK)
    {
        printf("和redis断开失败 error:%s", pstContext->errstr);
        return;
    }
    printf("和redis断开成功!\n");
}



void RedisChannel::RedisAuthCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult , void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    
    //FIXME All pending callbacks are called with a NULL reply when the context encountered an error.
    //
    if(pstRedisReply)
    {
        // 成功
        printf("鉴权结果:%s\n", pstRedisReply->str);
    }
    else
    {
        // 失败
        if(pstRedisAsyncContext->err)
        {
            printf("鉴权发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("鉴权发生未知错误\n");
        }

        return;   // 直接返回,自动清理
    }
     
  
    if(static_cast<RedisChannel*>(pstData)->IsSub())
    {
        printf("启动订阅\n");
        if(::redisAsyncCommand(pstRedisAsyncContext, RedisSubMsgCallback, pstData, "SUBSCRIBE dabyang") != REDIS_OK)
        {
            if(pstRedisAsyncContext->err)
            {
                printf("订阅启动发生错误:%s\n", pstRedisAsyncContext->errstr);
            }
            else
            {
                printf("订阅启动发生未知错误\n");
            }
        }

    }
}

void RedisChannel::RedisSubMsgCallback(::redisAsyncContext* pstRedisAsyncContext, void* pstResult, void* pstData)
{
    ::redisReply* pstRedisReply = static_cast<::redisReply*>(pstResult);
    if(pstRedisReply)
    {
        if(pstRedisReply->type == REDIS_REPLY_ARRAY)
        {
            for(int i = 0; i < static_cast<int>(pstRedisReply->elements); i++)
            {
                printf("%u) %s\n", i, pstRedisReply->element[i]->str);
                
                // TODO 消息解析器 
                // 消息ID: 判断消息缺损
                // openid: 玩家的openid
                // time: 玩家数据到期时间戳
            }

            if(pstRedisReply->element[2]->str)
            {
                printf("dd:%s\n",pstRedisReply->element[2]->str);


                int a[4];
                a[0] = 0;
                a[3] = pstRedisReply->element[2]->len;
                
                 printf("ddddc\n");
                 int iNext = 0;  
                for(int i = 0; i<2 ; i++)
                {   
                    char* pstAddress = ::strchr(pstRedisReply->element[2]->str + (iNext+1), '_');
              
                    if(pstAddress)
                        iNext = static_cast<int>(pstAddress - pstRedisReply->element[2]->str);

                    a[i+1] = iNext;
                }

                printf("dddd\n");
                char ID[15]={0};
                char openid[40]={0};
                char expireTime[15]={0};


                ::strncpy(ID, pstRedisReply->element[2]->str + a[0], a[1]);
                ::strncpy(openid, pstRedisReply->element[2]->str + a[1]+1, a[2]-a[1]-1);
                ::strncpy(expireTime, pstRedisReply->element[2]->str + a[2]+1, a[3]-a[2]-1);
                

                printf("ExpireTime: %s\n", expireTime);
                printf("Openid: %s\n", openid);
                printf("ID: %s\n", ID);
                
                
                auto p1 = static_cast<RedisChannel*>(pstData);


                do
                {
                    // TODO 用简单可靠的消息队列代替
                    if(p1->Seq() + 1 == static_cast<uint32_t>(::atoi(ID)))
                    {
                        // 正确的消息
                        // TODO 直接注册
                    
                        p1->SetSeq(p1->Seq() + 1);
                       // p1->AddTimer(::atoi(expireTime), ::atoi(ID), std::move(std::string(openid)));
                    
                        // TODO 检查holdback队列
                        uint32_t iReady = p1->Seq();
                    
                        while(!p1->IsHoldBackQueueEmpty())
                        {
                            iReady++;
                            auto pst = p1->HoldBackQueueMsgBy(iReady); //+1
                            if(pst)
                            {
                              //  p1->AddTimer(pst->m_ulExpireTime, pst->m_dwMsgID, std::move(pst->m_strOpenid));
                                //TODO 成功传递到了 写个日志,发消息来干掉redis里面back消息id数据(这个时候奔溃了,可以根据日志恢复，redis里的back无所的)
                                p1->HoldBackQueueEraseBy(iReady);
                                p1->SetSeq(iReady);  // case 1: HoldBackQueue为空     case 2: 还有缺
                            }
                            else
                            {
                                //还有缺
                                break;
                            }
                            // -1
                        }

                        if(p1->IsHoldBackQueueEmpty())
                        {
                            break;          // 序号正确
                        }

                    }   
                

                    if(p1->Seq() + 1 < static_cast<uint32_t>(::atoi(ID)))
                    {
                        // 遗漏了一些消息
                        // TODO 1. 将这些消息丢到接受队列中  2. 去拿redis遗漏的id对应的消息

                        std::shared_ptr<dan::eventloop::Msg> pstMsg(new dan::eventloop::Msg(static_cast<uint32_t>(::atoi(ID)), std::move(std::string(openid)), ::atoi(expireTime)));
                        p1->AddToHoldBackQueue(static_cast<uint32_t>(::atoi(ID)), pstMsg);


                        // TODO 2. 去拿redis遗漏的ids对应的消息并设置seq   go这边是一个事务:保证数据不丢，写到redis里
                        //         在redis list中(全部拿)
                        


                    }
                
                    if(p1->Seq() + 1 > static_cast<uint32_t>(::atoi(ID)))
                    {
                        // 重复的消息
                        // TODO 丢弃这个消息
                    }
                }while(false);



               // p1->AddTimer(::atoi(expireTime), ::atoi(ID), std::move(std::string(openid)));
                
            }

        }
    }
    else
    {
        if(pstRedisAsyncContext->err)
        {
            printf("订阅消息发生错误:%s\n", pstRedisAsyncContext->errstr);
        }
        else
        {
            printf("订阅消息发生未知错误\n");
        }
    }
}




}
}
