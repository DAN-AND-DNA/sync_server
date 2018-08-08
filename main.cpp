#include <sync_server/SyncServer.h>
#include <sync_server/eventloop/EventLoop.h>
#include <signal.h>
namespace
{

dan::eventloop::EventLoop stEventloop; 

std::shared_ptr<dan::SyncServer> pstSyncServer(new dan::SyncServer(&stEventloop));

typedef void(*TFuncptr)(int);

void StopTimers(int)
{
    printf("准备停止定时器\n");
    if(!pstSyncServer->IsTimerRun())
    {
        stEventloop.StopLoop();
    }
    else
    {
        printf("=====================try again=====================\n");
    }
}

TFuncptr pstFunc = StopTimers; 

}


int main()
{

    ::signal(SIGPIPE, SIG_IGN);
    ::signal(SIGINT, pstFunc);

    // in stack
    // one eventloop per thread
    //dan::eventloop::EventLoop stEventloop;

    if(!stEventloop.IsInit())
        return -1;

    //std::shared_ptr<dan::SyncServer> pstSyncServer(new dan::SyncServer(&stEventloop));

    if(!pstSyncServer->IsInit())
        return -1;

    pstSyncServer->Run();
}
