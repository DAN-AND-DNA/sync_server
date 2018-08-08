#include <sync_server/eventloop/Channel.h>
#include <sync_server/eventloop/EventLoop.h>
//#include <sync_server/net/Conn.h>

#include <iostream>
#include <sys/epoll.h>

namespace dan
{
namespace eventloop
{

const int Channel::s_iNoneEvent = 0;
const int Channel::s_iReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::s_iWriteEvent = EPOLLOUT;

Channel::Channel(int fd,  eventloop::EventLoop* pstOwnerEventLoop, bool bIsServerChannel):
    m_iFd_(fd),
    m_iStatus_(-1),
    m_iEvents_(0),
    m_iRevents_(0),
    m_pstOwnerEventLoop_(pstOwnerEventLoop),
    m_bIsServerChannel_(bIsServerChannel)
{}

Channel::~Channel()
{
    printf("close channel\n");
}

void Channel::Tie(std::shared_ptr<dan::net::Conn>& pstConn)
{
    m_pstConn_ = pstConn;
}

void Channel::Tie(std::shared_ptr<void>& pstConn)
{
    m_pstConn_ = pstConn;
}

void Channel::HandleEvent()
{
    if(m_bIsServerChannel_ || m_pstConn_.lock())
    {
        if((m_iRevents_ & EPOLLHUP) && m_stCloseCallback_) m_stCloseCallback_();                     // 客户端断开

        if((m_iRevents_ & EPOLLERR) && m_stErrorCallback_) m_stErrorCallback_();                     // 发生错误

        if((m_iRevents_ & (EPOLLIN | EPOLLPRI)) && m_stReadCallback_) m_stReadCallback_();           // 可读

        if((m_iRevents_ & EPOLLOUT) && m_stWriteCallback_) m_stWriteCallback_();                     // 可写
    }
}

void Channel::UpdateToEventLoop()
{

    if(m_pstOwnerEventLoop_)
        m_pstOwnerEventLoop_->UpdateChannel(this);
}


}
}
