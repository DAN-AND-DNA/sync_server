#pragma once


#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


namespace dan
{
namespace net
{

class SocketWrapper
{
public:
    static const int CSOCKET;

    static const int SSOCKET; 

    SocketWrapper(int iSocketType, int iPort = 7777, const char* szAddress = nullptr) noexcept;      

    ~SocketWrapper() noexcept;

    int Fd(){return m_iFd_;}
    
    bool IsInit(){return m_bIsInit_;}
    
    int Port(){return m_iPort_;}

private:
    int ServerSocket();

private:
    int m_iPort_;
    int m_iType_;
    int m_iFd_;                 // socket fd
    bool m_bIsInit_;

};



}
}
