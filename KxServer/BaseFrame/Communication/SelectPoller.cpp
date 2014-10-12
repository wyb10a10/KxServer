#include "SelectPoller.h"

using namespace std;

namespace KxServer {

CSelectPoller::CSelectPoller(void)
{
	m_TimeOut.SetFromfloat(0.00001f);
	FD_ZERO(&m_InSet);
	FD_ZERO(&m_OutSet);
}

CSelectPoller::~CSelectPoller(void)
{
}

int CSelectPoller::Poll()
{
    //clear the poll object need to be remove
    Clear();

	//m_InSet, m_OutSet, m_ErrSet Manage By Poller
	fd_set inset = m_InSet;
	fd_set outset = m_OutSet;
	TimeVal t = m_TimeOut;
	int ret = select(m_MaxCount, &inset, &outset, NULL, (timeval*)&(t));

    //if the socket faile, windows will return -1, if nothing happen, return 0
    if (ret > 0 || ret == -1)
	{
		//notify event count
		int eventCounts = ret;

		//process
		for (set<ICommunication*>::iterator iter = m_AllSet.begin();
			iter != m_AllSet.end() && eventCounts > 0; )
		{
			ICommunication* obj = *iter;
			COMMUNICATIONID cid = obj->GetCommunicationID();

            do 
            {
                if(FD_ISSET(cid, &inset))
                {
                    //process data in
                    --eventCounts;
                    if (0 > obj->OnRecv())
                    {
                        RemovePollObject(obj);
                        //will be release
                        obj->OnError();
                        break;
                    }
                }

                if(FD_ISSET(cid, &outset))
                {
                    //process data out
                    --eventCounts;
                    if (0 > obj->OnSend())
                    {
                        RemovePollObject(obj);
                        //will be release
                        obj->OnError();
                        break;
                    }
                }
            } while (false);

			++iter;
		}
	}

	return ret;
}

int CSelectPoller::AddPollObject(ICommunication* obj, int type)
{
	COMMUNICATIONID id = obj->GetCommunicationID();

#ifndef WIN32
	if(m_MaxCount <= id)
	{
		m_MaxCount = id + 1;
	}
#endif

	if(type & POLLTYPE_IN )
	{
		FD_SET(id, &m_InSet);
	}

	if(type &  POLLTYPE_OUT)
	{
		FD_SET(id, &m_OutSet);
	}

	m_AllSet.insert(obj);
	obj->SetPollType(type);
    obj->SetPoller(this);

	return 0;
}

int CSelectPoller::ModifyPollObject(ICommunication* obj, int type)
{
	if (m_AllSet.find(obj) == m_AllSet.end())
	{
		// not find
		return -1;
	}

	//remove - add
	if (type & POLLTYPE_IN)
	{
		if (!(obj->PollType() & POLLTYPE_IN))
		{
			FD_SET(obj->GetCommunicationID(), &m_InSet);
		}
	}
	else
	{
		FD_CLR(obj->GetCommunicationID(), &m_InSet);
	}

	if (type & POLLTYPE_OUT)
	{
		if (!(obj->PollType() & POLLTYPE_OUT))
		{
			FD_SET(obj->GetCommunicationID(), &m_OutSet);
		}
	}
	else
	{
		FD_CLR(obj->GetCommunicationID(), &m_OutSet);
	}

	obj->SetPollType(type);

	return 0;
}

int CSelectPoller::RemovePollObject(ICommunication* obj)
{
	m_RemoveSet.insert(obj);
    m_RemoveIdSet.insert(obj->GetCommunicationID());
	return 0;
}

void CSelectPoller::Clear()
{
	for (set<COMMUNICATIONID>::iterator iter = m_RemoveIdSet.begin();
		iter != m_RemoveIdSet.end(); ++iter)
	{
		COMMUNICATIONID cid = *iter;
		FD_CLR(cid, &m_InSet);
		FD_CLR(cid, &m_OutSet);
	}

    for (set<ICommunication*>::iterator iter = m_RemoveSet.begin();
        iter != m_RemoveSet.end(); ++iter)
    {
        m_AllSet.erase(*iter);
    }

	m_RemoveSet.clear();
    m_RemoveIdSet.clear();
}

}
