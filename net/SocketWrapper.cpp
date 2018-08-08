#include <sync_server/net/SocketWrapper.h>

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

namespace dan
{
namespace net
{

const int SocketWrapper::CSOCKET = 0;

const int SocketWrapper::SSOCKET = 1;


SocketWrapper::SocketWrapper(int iSocketType,int iPort, const char* szAddress) noexcept:
    m_iPort_(iPort),
    m_iType_(iSocketType),
    m_iFd_(-1),
    m_bIsInit_(false)
    {
        switch(iSocketType)
        {
            case SSOCKET:
            {
                // 服务器socket
                struct addrinfo stHints;
                struct addrinfo* pstResult;

                ::bzero(&stHints, sizeof stHints);
                stHints.ai_family = AF_INET;
                stHints.ai_socktype = SOCK_STREAM;
                stHints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
             

                char szPort[6] = {0};
                ::snprintf(szPort, 6, "%d", iPort);      // 6-1个字符

                if(::getaddrinfo(szAddress, szPort, &stHints, &pstResult) !=0)
                {
                    // TODO 打印错误
                    break;
                }

                for(struct addrinfo* pstCurr = pstResult; pstCurr != nullptr; pstCurr = pstResult->ai_next)
                {
                    m_iFd_ = ::socket(pstCurr->ai_family, pstCurr->ai_socktype, pstCurr->ai_protocol);
                    int iOptval = 1;
                    if(m_iFd_ == -1 || ::setsockopt(m_iFd_, SOL_SOCKET, SO_REUSEADDR, &iOptval, sizeof iOptval) == -1|| 
                                        ::bind(m_iFd_, pstResult->ai_addr, pstResult->ai_addrlen) == -1 || 
                                        ::listen(m_iFd_, 64) == -1)
                    {
                        //TODO 打印错误
                        if(m_iFd_ != -1)
                            ::close(m_iFd_);
                        continue;
                    }
                    else
                    {
                        m_bIsInit_ = true;
                    }
                }
                
                if(pstResult != nullptr)
                {
                    ::freeaddrinfo(pstResult);
                    pstResult = nullptr;

                }
                break;
            }
            case CSOCKET:
            {
                // 客户端socket
                struct addrinfo stHints;
                struct addrinfo* pstResult;

                ::bzero(&stHints, sizeof stHints);
                stHints.ai_family = AF_INET;
                stHints.ai_socktype = SOCK_STREAM;
                stHints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
             

                char szPort[6] = {0};
                ::snprintf(szPort, 6, "%d", iPort);      // 6-1个字符

                if(::getaddrinfo(szAddress, szPort, &stHints, &pstResult) !=0)
                {
                    // TODO 打印错误
                    break;
                }

                for(struct addrinfo* pstCurr = pstResult; pstCurr != nullptr; pstCurr = pstResult->ai_next)
                {
                    m_iFd_ = ::socket(pstCurr->ai_family, pstCurr->ai_socktype, pstCurr->ai_protocol);
                    int iOptval = 1;
                    if(m_iFd_ == -1 || ::setsockopt(m_iFd_, SOL_SOCKET, SO_REUSEADDR, &iOptval, sizeof iOptval) == -1)
                    {
                        //TODO 打印错误
                        if(m_iFd_ != -1)
                            ::close(m_iFd_);
                        continue;
                    }
                    else
                    {
                        m_bIsInit_ = true;
                    }
                }
                
                if(pstResult != nullptr)
                {
                    ::freeaddrinfo(pstResult);
                    pstResult = nullptr;

                }

                break;
            }
            default:
            {
                break;
            }
        }
    }


SocketWrapper::~SocketWrapper() noexcept
{
    if(m_iFd_ != -1)
        ::close(m_iFd_);

    m_bIsInit_ = false;
}


}
}
