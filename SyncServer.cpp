#ifdef __cplusplus
extern "C" {
#endif

#include <sync_server/redis/hiredis.h>
#include <sync_server/redis/async.h>

#ifdef __cplusplus 
}
#endif





#include <sync_server/SyncServer.h>
#include <sync_server/eventloop/EventLoop.h>
#include <sync_server/net/SocketWrapper.h>
#include <sync_server/eventloop/Channel.h>
#include <sync_server/net/Conn.h>
//#include <sync_server/mod/Mod.h>
#include <sync_server/timer/Timer.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <functional>

#include <iostream>  // for easy log


namespace dan
{

const int SyncServer::SERVER_DONE = 0;
const int SyncServer::COMPONENT_INIT_FAILED = -1;
const int SyncServer::ACCEPT_ERROR = -2;


SyncServer::SyncServer(eventloop::EventLoop* pstEventLoop) noexcept: 
    m_pstEventLoop_(pstEventLoop),
    m_pstServerSocket_(new net::SocketWrapper(net::SocketWrapper::SSOCKET)),
    m_pstServerChannel_(new eventloop::Channel(m_pstServerSocket_->Fd(), pstEventLoop, true)),
    m_bIsInit_(false),
    m_stConnsMap_()
{
   // dan::mod::Mod::LoadMsg();
    if(m_pstServerSocket_->IsInit() && m_pstEventLoop_ != nullptr &&m_pstEventLoop_->IsInit() && m_pstServerChannel_->IsInit())//&& m_pstRedisChannel_->IsInit(), m_pstRedisSubChannel_->IsInit())
    {
            m_bIsInit_ = true;
    }

}


SyncServer::~SyncServer(){}


int SyncServer::Run() noexcept
{

    if(!m_bIsInit_)
        return COMPONENT_INIT_FAILED; 
   
    AddTimer(40, -1, "server", false);

    m_pstServerChannel_->SetReadCallback(std::bind(&SyncServer::AcceptCallback, this));//,std::weak_ptr<SyncServer>(shared_from_this())));
    m_pstServerChannel_->EnableRead();

    
    

    // looping....
    m_pstEventLoop_->Loop();

    return SERVER_DONE;

}

int SyncServer::Fd()
{
    return m_pstServerSocket_->Fd();
}

bool SyncServer::IsTimerRun(){return m_stTimersMap_["server"]->IsRun();}

void SyncServer::AddTimer(uint64_t ulExpireTime, int iID, std::string&& strOpenid, bool bIsUser)
{
    std::shared_ptr<dan::SyncServer> pst = shared_from_this(); //+1    
    std::string strName(strOpenid);
    std::shared_ptr<dan::timer::Timer> pstTimer(new dan::timer::Timer(ulExpireTime, std::move(strOpenid), EventLoopPtr(), pst, bIsUser));
    pstTimer->Init();

    if(pstTimer->IsInit())
    {
        m_stTimersMap_[strName] = pstTimer;
    }

    std::cout<<"当前time size:"<<m_stTimersMap_.size()<<std::endl;
}

void SyncServer::RemoveTimer(std::string&& strOpenid)
{
    std::cout<<strOpenid<<std::endl;
   auto it = m_stTimersMap_.find(strOpenid);
   if(it != m_stTimersMap_.end())
   {
       printf("delete timer\n");
       m_stTimersMap_.erase(it);
   }
}

void SyncServer::GetUser(std::string& strOpenid)
{
    //m_pstRedisChannel_->GetUser(strOpenid);
}

void SyncServer::GetUserCount()
{
   // m_pstRedisChannel_->GetUserCount();
}


void SyncServer::AcceptCallback()
{
    std::cout <<"a client\n";
    
    struct sockaddr_in stClientAddr;
    socklen_t stSocklen = sizeof stClientAddr;
    void* pstTemp = static_cast<void*>(&stClientAddr);


    while(true)
    {
        int iFd = ::accept4(Fd(),
                            static_cast<struct sockaddr*>(pstTemp),
                            &stSocklen,
                            SOCK_CLOEXEC|SOCK_NONBLOCK);

        if(iFd == -1)
        {
            if(errno == EINTR)
                continue;

            std::cout<<"accept failed!  err:"<<::strerror(errno);
            return;
        }
        else
        {
            std::cout<<"new client ip:"<<::inet_ntoa(stClientAddr.sin_addr)<<" port:"<<ntohs(stClientAddr.sin_port)<<std::endl;

            std::shared_ptr<dan::SyncServer> pst = shared_from_this(); //+1 
            std::shared_ptr<net::Conn> pstConn(new net::Conn(iFd, EventLoopPtr(), pst));   // stack 
            pstConn->Init();


            std::cout<<"new fd:"<<iFd<<std::endl;
            m_stConnsMap_[iFd] = pstConn;                                // keep 1
        }
        break;
    }
    
}


}
