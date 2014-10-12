#include "CommPool.h"

using namespace std;

namespace KxServer {

CCommPool* CCommPool::m_Instance = NULL;

CCommPool::CCommPool(void)
{
}

CCommPool::~CCommPool(void)
{
	for (map<COMMUNICATIONID, ICommunication*>::iterator iter = m_CommMap.begin();
		iter != m_CommMap.end(); ++iter)
	{
		delete iter->second;
	}

	m_CommMap.clear();
}

//ergodic CommMap, check they tag, and send data
void CCommPool::BroadcastByTag(int tag, char* buffer, unsigned int len)
{
	for (map<COMMUNICATIONID, ICommunication*>::iterator iter = m_CommMap.begin();
		iter != m_CommMap.end(); ++iter)
	{
		if (iter->second->GetTag() == tag)
		{
			iter->second->Send(buffer, len);
		}
	}
}

//get communcation by id
/*ICommunication* CCommPool::GetCommuncation(COMMUNICATIONID id)
{
	map<COMMUNICATIONID, ICommunication*>::iterator iter = m_CommMap.find(id);
	if (iter != m_CommMap.end())
	{
		return iter->second;
	}

	return NULL;
}*/

//add communcation
bool CCommPool::AddCommuncation(ICommunication* obj)
{
	//if no check, will cause memery leak
	if (NULL != obj 
		&& m_CommMap.end() == m_CommMap.find(obj->GetCommunicationID()))
	{
		m_CommMap[obj->GetCommunicationID()] = obj;
		return true;
	}

	return false;
}

//remove and delete communcation
bool CCommPool::RemoveCommuncation(COMMUNICATIONID id)
{
	map<COMMUNICATIONID, ICommunication*>::iterator iter = m_CommMap.find(id);
	if (iter != m_CommMap.end())
	{
        ICommunication* obj = iter->second;
        m_CommMap.erase(iter);
        delete obj;
		return true;
	}
	else
	{
		return false;
	}
}

}
