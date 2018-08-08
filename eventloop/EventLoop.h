#pragma once

#include <vector>
#include <memory>



namespace dan
{
namespace eventloop
{

class Channel;  // channel 代表一个对fd监听通道

class IPoller;    // 接口 底层可以是poll 或 epoll


// 事件循环
class EventLoop
{
public:
    EventLoop();

    virtual ~EventLoop();

    void Loop();

    bool IsInit(){return m_bIsInit_;}

    void UpdateChannel(Channel* pstChannel);//{pstPoller_->UpdateChannel(pstChannel);}
    
    void RemoveChannel(Channel* pstChannel);

    void StopLoop();
private:
    typedef std::vector<Channel*> TChannelList;

    TChannelList m_stActiveChannels_;               // 每次有消息到底的通道

    Channel* m_pstCurrChannel_;                     // 

    bool m_bIsStop_;                                // 停止事件循环

    std::unique_ptr<IPoller> pstPoller_;            // 

    bool m_bIsInit_;                                // 初始成功?
};


}
}
