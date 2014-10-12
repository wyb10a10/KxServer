/*
 * CommPool 通讯对象池
 * 管理通讯对象，向通讯对象发送数据
 *
 * 2013-04-12 By 宝爷
 * 
 */
#ifndef __SOCKETPOOL_H__
#define __SOCKETPOOL_H__

#include <map>
#include <vector>
#include "ICore.h"

namespace KxServer {

class CCommPool
{
private:
	CCommPool(void);
	~CCommPool(void);

public:
	static CCommPool* GetInstance()
	{
		if (NULL == m_Instance)
		{
			m_Instance = new CCommPool();
		}
		return m_Instance;
	}

	static void Destroy()
	{
		delete m_Instance;
		m_Instance = NULL;
	}

	// m_Tag + 1
	inline int IncrTag(){ return ++m_Tag; }

	//return m_Tag
	inline int GetTag(){ return m_Tag; }

	//ergodic CommMap, check they tag, and send data
	//if you have two tcp listener and many tcp clienter, this function will be slow
	void BroadcastByTag(int tag, char* buffer, unsigned int len);

	//get communcation by id
	inline ICommunication* GetCommuncation(COMMUNICATIONID id)
    {
        std::map<COMMUNICATIONID, ICommunication*>::iterator iter = m_CommMap.find(id);
        if (iter != m_CommMap.end())
        {
            return iter->second;
        }

        return NULL;
    }

	//add communcation
	bool AddCommuncation(ICommunication* obj);

	//remove and delete communcation
	bool RemoveCommuncation(COMMUNICATIONID id);

private:
	static CCommPool*							m_Instance;
	int											m_Tag;
	std::map<COMMUNICATIONID, ICommunication*>	m_CommMap;
	std::map<int, ICommunication*>				m_TagMap;
};

}

#endif
