#pragma once

#include <sync_server/eventloop/IPoller.h>


struct epoll_event;

//class Channel;

namespace dan
{
namespace eventloop
{

class EPoller : public IPoller
{
public:
    EPoller(EventLoop* pstOwnerEventLoop);

    virtual ~EPoller();

    virtual void Poll(int iWaitingMs, TChannelList* pstActiveChannels);             //

    virtual void UpdateChannel(Channel* pstChannel);

private:
    void FillActiveChannels(int iNumEvents, TChannelList* pstActiveChannels);

    void UpdateToEpoll(int iOP, Channel* pstChannel);
private:
    static const int s_iWaittingEventListSize_ = 32;

    typedef std::vector<struct epoll_event> TWaittingEventList;

    int m_iFd_;             // epoll fd

    TWaittingEventList  m_stWaittingEventList_;


};

}
}
