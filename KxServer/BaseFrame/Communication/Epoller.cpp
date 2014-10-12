#include "Epoller.h"
#include "CommPool.h"

namespace KxServer {

#ifndef WIN32

CEpoller::CEpoller(int maxEventQueue)
:m_MaxEventQueue(maxEventQueue)
{
	m_TimeOut = 1;
	m_EpollFd = epoll_create(maxEventQueue);
}

CEpoller::~CEpoller(void)
{
	shutdown(m_EpollFd, SHUT_RDWR);
}

int CEpoller::Poll()
{
	int maxnotify = epoll_wait(m_EpollFd, m_Events, m_MaxEventQueue, m_TimeOut);
	for (int i = 0; i < maxnotify; ++i)
	{
		ICommunication* obj = CCommPool::GetInstance()->GetCommuncation(m_Events[i].data.fd);
		if (NULL == obj)
		{
			continue;
		}

		if (m_Events[i].events & EPOLLIN)
		{
			if(0 > obj->OnRecv())
            {
                RemovePollObject(obj);
                obj->OnError();             
                continue;
            }
		}

		if (m_Events[i].events & EPOLLOUT)
		{
			if(0 > obj->OnSend())
            {
                RemovePollObject(obj);
                obj->OnError();   
                continue;
            }
		}
	}
}

int CEpoller::AddPollObject(ICommunication* obj, int type)
{
	struct epoll_event ev;
	ev.data.fd = obj->GetCommunicationID();
	ev.events  = type | EPOLLET;
	return epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, ev.data.fd, &ev);
}

int CEpoller::ModifyPollObject(ICommunication* obj, int type)
{
	struct epoll_event ev;
	ev.data.fd = obj->GetCommunicationID();
	ev.events  = type | EPOLLET;
	return epoll_ctl(m_EpollFd, EPOLL_CTL_MOD, ev.data.fd, &ev);
}

int CEpoller::RemovePollObject(ICommunication* obj)
{
	struct epoll_event ev;
	ev.data.fd = obj->GetCommunicationID();
	return epoll_ctl(m_EpollFd, EPOLL_CTL_DEL, ev.data.fd, &ev);

	return -1;
}

#endif

}
