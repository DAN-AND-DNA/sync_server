#include <sync_server/eventloop/Channel.h>
//#include <sync_server/mod/Mod.h>
#include <sync_server/eventloop/EventLoop.h>
#include <sync_server/net/Conn.h>
#include <sync_server/SyncServer.h>


#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>


namespace dan
{
namespace net
{

Conn::Conn(int iFd, dan::eventloop::EventLoop* pstEventLoop, std::shared_ptr<dan::SyncServer>& pstServer):
    m_iFd_(iFd),
    m_stInBuffer_(1440),
    m_stOutBuffer_(1440),
    m_pstChannel_(new dan::eventloop::Channel(iFd, pstEventLoop)),
    //m_pstMod_(new dan::mod::Mod()),
    m_pstServer_(pstServer),
    m_iInBufferSize_(0),
    m_iOutBufferSize_(0)
{
    m_pstChannel_->SetReadCallback(std::bind(&Conn::RecvCallback, this));
    m_pstChannel_->SetWriteCallback(std::bind(&Conn::SendCallback, this));
    m_pstChannel_->SetCloseCallback(std::bind(&Conn::CloseCallback, this));

    m_pstChannel_->DisableWrite();
    m_pstChannel_->EnableRead();

    std::cout<<m_stInBuffer_.size()<<std::endl;
}

Conn::~Conn()
{
    if(m_iFd_ > 0)
        ::close(m_iFd_);
}

void Conn::Init()
{
    if(m_pstChannel_->IsInit())
    {
        std::shared_ptr<Conn> pst = shared_from_this(); //+1

        m_pstChannel_->Tie(pst);

       // m_pstMod_->Tie(pst);

        // -1
    }
}

void Conn::EnableWrite()
{
    m_pstChannel_->EnableWrite();
}

void Conn::DisableWrite()
{
    m_pstChannel_->DisableWrite();
}

void Conn::RecvCallback()
{
    if(m_pstServer_.lock())
    {
        size_t dwLength = 1440;
        int iAllInSize = InBufferSize();
        ssize_t iSize = ::recv(m_iFd_, InBufferPtr(iAllInSize), dwLength, 0);

        if(iSize == 0)
        {
            ::close(m_iFd_);
            CloseCallback();
            std::cout<<"close client\n";
            // TODO 客户端关闭
        }
        else if(iSize > 0)
        {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);
            printf("in recvcall:%ld\n", tv.tv_sec*1000000 + tv.tv_usec);

            SetInBufferSize(iAllInSize + static_cast<int>(iSize));

            std::cout<<"get size:"<<InBufferSize()<<std::endl;
        
         //   m_pstMod_->HandleArrivedMsg();
        }
        else if(errno != EAGAIN)
        {
            // TODO 发生错误
        }
    }
}

void Conn::SendCallback()
{
    if(m_pstServer_.lock())
    {
        //size_t dw
        std::cout<<"call sendcall:"<<OutBufferSize()<<std::endl;
        if(OutBufferSize() <= 0)
            return;

        int iSended = static_cast<int>(::send(Fd(), OutBufferPtr(0), OutBufferSize(), 0));

        if(iSended > 0)
        {
            std::cout<<"poll发送:"<<iSended<<std::endl;

            int iLeftSize = OutBufferSize() - iSended;
            SetOutBufferSize(iLeftSize);
            if(iLeftSize == 0)
            {
                struct timeval tv;
                ::gettimeofday(&tv, NULL);
                printf("发送完毕:%ld\n", tv.tv_sec*1000000 + tv.tv_usec);

                SetOutBufferSize(0);
                DisableWrite();
            }
            else if(iLeftSize > 0)
            {
                ::memcpy(OutBufferPtr(0), OutBufferPtr(iSended), iLeftSize);
                SetOutBufferSize(iLeftSize);
            }
        }
        else if(iSended == 0)
        {
            // nothing
        }
        else if(errno != EAGAIN)
        {
        // TODO error
        }
    }

}

void Conn::CloseCallback()
{
    if(auto pstS = m_pstServer_.lock())
    {
        m_pstChannel_->DisableAll();
        auto it = (pstS->ConnsMap())->find(m_iFd_);

            //std::cout<<"清理 conn"<<(it->second).use_count()<<std::endl;
        std::cout<<"清理 conn\n";
        (pstS->ConnsMap())->erase(it);
    }

}



}
}
