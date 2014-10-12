/*
 * 实现跨平台的Select
 * 在每次Poll轮询的时候，会将异常的对象添加到删除列表中
 * 在轮询完之后，一次性删除
 *
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __SELECTPOLLER_H__
#define __SELECTPOLLER_H__

#include <set>

#include "ICore.h"

namespace KxServer {

class CSelectPoller :
	public ICommunicationPoller
{
public:
	CSelectPoller(void);
	virtual ~CSelectPoller(void);

	//poll all communication object, and process event
	virtual int Poll();

	virtual int AddPollObject(ICommunication* obj, int type);

	virtual int ModifyPollObject(ICommunication* obj, int type);
	
    virtual int RemovePollObject(ICommunication* obj);

private:
	void Clear();

private:
	fd_set	m_InSet;
	fd_set	m_OutSet;
	TimeVal m_TimeOut;
	int		m_MaxCount;

	std::set<ICommunication*> m_AllSet;
    std::set<ICommunication*> m_RemoveSet;
    std::set<COMMUNICATIONID> m_RemoveIdSet;
};

}

#endif
