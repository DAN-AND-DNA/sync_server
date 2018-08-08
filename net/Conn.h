#pragma once

#include <memory>
#include <vector>

//#include <sync_server/mod/Protocol.h>


namespace dan
{

class SyncServer;

namespace mod
{
//class Mod;
}

namespace eventloop
{
class Channel;
class EventLoop;
}


namespace net
{

class Conn : public std::enable_shared_from_this<Conn>
{
public:

    Conn(int iFd, dan::eventloop::EventLoop* pstEventLoop, std::shared_ptr<dan::SyncServer>& pstServer);

    ~Conn();    // 需要 unique_ptr<T> 其中T必须完整

    void Init();

    uint8_t* InBufferPtr(int iSize){return &m_stInBuffer_[iSize];}

    uint8_t* OutBufferPtr(int iSize){return &m_stOutBuffer_[iSize];}

    int InBufferSize(){return m_iInBufferSize_;}

    int OutBufferSize(){return m_iOutBufferSize_;}

    void SetInBufferSize(int iSize){m_iInBufferSize_ = iSize;}

    void SetOutBufferSize(int iSize){m_iOutBufferSize_ = iSize;}

    void EnableWrite();

    void DisableWrite();

    int Fd(){return m_iFd_;}

//    void Clear(){m_stInBuffer_.clear();}

//    int Count(){return static_cast<int>(m_stInBuffer_.size());}
private:
    void RecvCallback();

    void SendCallback();

    void CloseCallback();
private:
    int m_iFd_;

    std::vector<uint8_t> m_stInBuffer_;         //FIXME 静态的,容易溢出

    std::vector<uint8_t> m_stOutBuffer_;

    std::unique_ptr<dan::eventloop::Channel> m_pstChannel_;
    
//    std::unique_ptr<dan::mod::Mod> m_pstMod_;

    std::weak_ptr<dan::SyncServer> m_pstServer_;

    int m_iInBufferSize_;

    int m_iOutBufferSize_;


};

}
}
