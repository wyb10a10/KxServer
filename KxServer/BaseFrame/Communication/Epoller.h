/*
 * 实现Linux下的Epoll
 *
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <set>
#include "ICore.h"

namespace KxServer {

#ifndef WIN32

#include <sys/epoll.h>
#define MAX_EPOLL_EVENTS_PER_RUN 100000

class CEpoller
	:public ICommunicationPoller
{
public:
	CEpoller(int maxEventQueue = MAX_EPOLL_EVENTS_PER_RUN);
	virtual ~CEpoller(void);

	//poll all communication object, and process event
	virtual int Poll();

	virtual int AddPollObject(ICommunication* obj, int type);
	virtual int ModifyPollObject(ICommunication* obj, int type);
	virtual int RemovePollObject(ICommunication* obj);

private:
	int m_EpollFd;
	int m_MaxEventQueue;
	int m_TimeOut;
	epoll_event m_Events[MAX_EPOLL_EVENTS_PER_RUN];
};

#endif

}

#endif
