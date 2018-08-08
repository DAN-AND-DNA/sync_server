#include <sync_server/net/SocketWrapper.h>
#include <iostream>


int main()
{
    dan::net::SocketWrapper stServerSocket(SSOCKET, 7777, nullptr);
    if(stServerSocket.IsInit())
    {
        std::cout<<"port:"<<stServerSocket.Port()<<" "<<"fd:"<<stServerSocket.Fd()<<std::endl;
    }
}
