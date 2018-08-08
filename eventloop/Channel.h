#pragma once

#include <functional>
#include <vector>
#include <memory>


namespace dan
{
namespace net
{
class Conn;
}

namespace eventloop
{

class EventLoop;

class Channel
{
public:

    typedef std::function<void()> TEventCallback;

    Channel(int fd, eventloop::EventLoop* pstOwnerEventLoop, bool bIsServerChannel = false);

    ~Channel();

    void Tie(std::shared_ptr<dan::net::Conn>& pstConn);

    void Tie(std::shared_ptr<void>& pstConn);

    bool IsInit(){return (m_iFd_ != -1 && m_pstOwnerEventLoop_);}

    void SetReadCallback(TEventCallback& stEventCallback){m_stReadCallback_ = stEventCallback;}
   
    void SetReadCallback(TEventCallback&& stEventCallback){m_stReadCallback_ = std::move(stEventCallback);}     //for std::bind
    
    void SetWriteCallback(TEventCallback& stEventCallback){m_stWriteCallback_ = stEventCallback;}

    void SetWriteCallback(TEventCallback&& stEventCallback){m_stWriteCallback_ = std::move(stEventCallback);}     //for std::bind
   
    void SetCloseCallback(TEventCallback& stEventCallback){m_stCloseCallback_ = stEventCallback;}
   
    void SetCloseCallback(TEventCallback&& stEventCallback){m_stCloseCallback_ = std::move(stEventCallback);}     //for std::bind
   
    void SetErrorCallback(TEventCallback& stEventCallback){m_stErrorCallback_ = stEventCallback;}

    int Fd(){return m_iFd_;}

    void SetFd(int iFd){m_iFd_ = iFd;}

    int Events(){return m_iEvents_;}

    void EnableRead() {m_iEvents_ |= s_iReadEvent; UpdateToEventLoop();}
    
    void EnableWrite() {m_iEvents_ |= s_iWriteEvent; UpdateToEventLoop();}

    void DisableWrite() {m_iEvents_ &= ~s_iWriteEvent; UpdateToEventLoop();}

    void DisableRead() {m_iEvents_ &= ~s_iReadEvent; UpdateToEventLoop();}

    void DisableAll() {m_iEvents_ = s_iNoneEvent; UpdateToEventLoop();}

    void SetRevents(int iRevents){ m_iRevents_ = iRevents;}

    int Status() {return m_iStatus_;}

    void SetStatus(int iStatus){m_iStatus_ = iStatus;}

    bool IsNoneEvent(){return m_iEvents_ == s_iNoneEvent;}

    void HandleEvent();
private:
    void UpdateToEventLoop();
private:
    static const int s_iNoneEvent;

    static const int s_iReadEvent;

    static const int s_iWriteEvent;

    TEventCallback m_stReadCallback_;
    
    TEventCallback m_stWriteCallback_;

    TEventCallback m_stCloseCallback_;

    TEventCallback m_stErrorCallback_;

    int m_iFd_;
    
    int m_iStatus_;                           // 通道的状态 
     
    int m_iEvents_;                           // 通道监听的事件

    int m_iRevents_;                          // 到达的事件

    eventloop::EventLoop* m_pstOwnerEventLoop_;          // 
    
    std::weak_ptr<void> m_pstConn_;

    bool m_bIsServerChannel_;
};

}
}
