#include <sync_server/eventloop/IPoller.h>
#include <sync_server/eventloop/EPoller.h>
#include <sync_server/eventloop/EventLoop.h>


namespace dan
{
namespace eventloop
{

IPoller::~IPoller(){}

IPoller* IPoller::NewPoller(EventLoop* pstEventLoop){return new EPoller(pstEventLoop);}

}
}
