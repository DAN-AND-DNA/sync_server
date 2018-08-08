#include <sync_server/eventloop/EventLoop.h>
#include <sync_server/eventloop/IPoller.h>
#include <sync_server/eventloop/Channel.h>


#include <iostream>
#include <sys/time.h>
#include <unistd.h>

namespace 
{
    const int iWaittingMs = 10000;         //10s
}

namespace dan
{
namespace eventloop
{

EventLoop::EventLoop():
    m_bIsStop_(false),
    pstPoller_(IPoller::NewPoller(this)),
    m_bIsInit_(false)
{
    if (pstPoller_ != nullptr)
        m_bIsInit_ = true;
}

EventLoop::~EventLoop(){}

void EventLoop::Loop()
{
   m_bIsStop_ = false;

   while(!m_bIsStop_)
   {
       pstPoller_->Poll(iWaittingMs, &m_stActiveChannels_);  //阻塞10s or 有事件触发
       
       if(m_stActiveChannels_.empty())
           continue;

       for(TChannelList::iterator it = m_stActiveChannels_.begin(); it < m_stActiveChannels_.end(); ++it)
       {
           m_pstCurrChannel_ = (*it);
           std::cout<<"handle\n";
           m_pstCurrChannel_->HandleEvent();
       }
       m_pstCurrChannel_ = nullptr;
       m_stActiveChannels_.clear();
   }
}

void EventLoop::UpdateChannel(Channel* pstChannel){pstPoller_->UpdateChannel(pstChannel);}

//void EventLoop::RemoveChannel()

void EventLoop::StopLoop()
{
    printf("开始关闭事件循环\n");
    m_bIsStop_ = true;
}



}
}
