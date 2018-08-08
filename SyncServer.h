#pragma once 

#include <memory>
#include <map>

class redisAsyncContext;


namespace dan
{


namespace eventloop
{
class EventLoop;
class Channel;
}

namespace net
{
class SocketWrapper;
class Conn;
}

namespace timer
{
class Timer;
}



class SyncServer: public std::enable_shared_from_this<SyncServer>
{
public:
     static const int SERVER_DONE;                    // 正常退出程序

     static const int COMPONENT_INIT_FAILED;          // 组件初始化失败

     static const int ACCEPT_ERROR;
public:
    SyncServer(eventloop::EventLoop* pstEventLoop) noexcept;
    
    ~SyncServer() noexcept;

    int Run() noexcept;

    bool IsInit() {return m_bIsInit_;}

    bool IsTimerRun();//{return m_stTimersMap_["server"]->IsRun();}

    int Fd();

    eventloop::EventLoop* EventLoopPtr(){return m_pstEventLoop_;}

    std::map<int, std::shared_ptr<dan::net::Conn>>* ConnsMap(){return &m_stConnsMap_;}
   
    void AddTimer(uint64_t ulExpireTime, int iID, std::string&& szOpenid, bool bIsUser = true);

    void RemoveTimer(std::string&& strOpenid);

    void GetUser(std::string& strOpenid);

    void GetUserCount();
private:
    void AcceptCallback();

private:
    eventloop::EventLoop*                    m_pstEventLoop_;              // 事件循环
    
    std::unique_ptr<net::SocketWrapper>      m_pstServerSocket_;           // 服务器socket
    
    std::unique_ptr<eventloop::Channel>      m_pstServerChannel_;
   
    bool                                     m_bIsInit_;

    std::map<int, std::shared_ptr<dan::net::Conn>> m_stConnsMap_;          // 复制 shared_ptr +1

    std::map<std::string, std::shared_ptr<dan::timer::Timer>> m_stTimersMap_;
};


}
