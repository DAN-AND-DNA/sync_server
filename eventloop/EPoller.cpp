#include <sync_server/eventloop/EPoller.h>
#include <sync_server/eventloop/Channel.h>
#include <iostream>

#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/time.h>


namespace
{
    const int iIsNew = -1;
    const int iIsAdded = 1;
    const int iIsDeleted = 2;
}

namespace dan
{
namespace eventloop
{

EPoller::EPoller(EventLoop* pstOwnerEventLoop):
    IPoller(pstOwnerEventLoop),
    m_iFd_(::epoll_create1(EPOLL_CLOEXEC)),
    m_stWaittingEventList_(s_iWaittingEventListSize_)
{}

EPoller::~EPoller()
{
    ::close(m_iFd_);
    //TODO 释放m_stWaittingEventList_
}


void EPoller::Poll(int iWaitingMs, TChannelList* pstActiveChannels)
{
    int iNumEvents = ::epoll_wait(m_iFd_, &(*(m_stWaittingEventList_.begin())), static_cast<int>(m_stWaittingEventList_.size()), iWaitingMs);

    if(iNumEvents > 0)
    {
       // std::cout<<"has events\n";
        
      //  struct timeval tv;
      //  ::gettimeofday(&tv, NULL);
    //    printf("in poll:%ld\n", tv.tv_sec*1000000 + tv.tv_usec);

        FillActiveChannels(iNumEvents, pstActiveChannels);
    }
    else if(iNumEvents == 0)
    {
        std::cout<<"no events\n";
        // 阻塞结束但没有事件
    }
    else if(errno != EINTR)
    {

         std::cout<<"err:"<<strerror(errno)<<std::endl;

        // 发生错误
    }

    //TODO 扩容
}

void EPoller::UpdateChannel(Channel* pstChannel)
{
    const int iStatus = pstChannel->Status();
    if (iStatus == iIsNew || iStatus == iIsDeleted)
    {
        std::cout<<"new chan\n";
        // 新增
        //m_stChannels_[pstChannel->Fd()] = pstChannel; 
        pstChannel->SetStatus(iIsAdded);
        UpdateToEpoll(EPOLL_CTL_ADD, pstChannel);
    }
    else
    {
        if (pstChannel->IsNoneEvent())
        {   // 删除
            UpdateToEpoll(EPOLL_CTL_DEL, pstChannel);
            pstChannel->SetStatus(iIsDeleted);
        }
        else
        {
            // 更改
            UpdateToEpoll(EPOLL_CTL_MOD, pstChannel);
        }

    }
}

void EPoller::FillActiveChannels(int iNumEvents, TChannelList* pstActiveChannels)
{
    for(int i = 0; i < iNumEvents; ++i)
    {
        Channel* pstChannel = static_cast<Channel*>(m_stWaittingEventList_[i].data.ptr);
        pstChannel->SetRevents(static_cast<int>(m_stWaittingEventList_[i].events));
        pstActiveChannels->push_back(pstChannel);
    }
}

void EPoller::UpdateToEpoll(int iOP, Channel* pstChannel)
{
    struct epoll_event stEvent;
    ::bzero(&stEvent, sizeof(stEvent));
    stEvent.events = pstChannel->Events();
    stEvent.data.ptr = pstChannel;

   // std::cout<<stEvent.events <<std::endl;
   // std::cout<<pstChannel->Fd() <<std::endl;

    if(::epoll_ctl(m_iFd_, iOP, pstChannel->Fd(), &stEvent) < 0)
    {
        // log
        std::cout<<"ctl err\n";
    }
}


}
}
