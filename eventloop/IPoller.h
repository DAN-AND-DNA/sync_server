#pragma once 

#include <map>
#include <vector>

namespace dan
{
namespace eventloop
{


class EventLoop;

class Channel;



// 接口
class IPoller
{
public:
    typedef std::vector<Channel*> TChannelList;                

    IPoller(EventLoop* pstOwnerEventLoop):
        m_pstOwnerEventLoop_(pstOwnerEventLoop){}

    virtual ~IPoller();

    virtual void Poll(int iWaitingMs, TChannelList* ChannelsOfTick) = 0;

    virtual void  UpdateChannel(Channel* pstChannel) = 0;

    static IPoller* NewPoller(EventLoop* pstEventLoop);
protected:
    typedef std::map <int, Channel*> TChannelMap;                       // <fd : channel*>

    //TChannelMap m_stChannels_;

private:
    EventLoop* m_pstOwnerEventLoop_;
};


}
}
