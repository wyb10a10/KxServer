/*
 * BufferList 缓冲区列表
 * 用于缓存数据块
 *
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __TCPCONNECTOR_H__
#define __TCPCONNECTOR_H__

#include "ICore.h"
#include "BaseSocket.h"
#include "BufferList.h"

namespace KxServer {

class CTCPConnector
	: public ICommunication
{
public:
	CTCPConnector(char* addr, int port, ICommunicationPoller* poller = NULL);
	virtual ~CTCPConnector(void);

	//call by user
	virtual int Send(char* buffer, unsigned int len);
	//call by framework
	virtual int Recv(char* buffer, unsigned int len);
	//get communication id, maybe fd int type int linux or SOCKET type in windows
	virtual COMMUNICATIONID GetCommunicationID();

	//call by poller
	virtual int OnRecv();

	//call by poller
	virtual int OnSend();
	
    //call by poller
	virtual int OnError();

    //call by user
    virtual void Close();

private:
	CBaseSocket* m_Socket;
	char* m_SendBuffer;
	char* m_RecvBuffer;
	unsigned int m_SendBufferLen;
	unsigned int m_RecvBufferLen;
	unsigned int m_SendBufferOffset;
	unsigned int m_RecvBufferOffset;
	CBufferList m_BufferList;

    static char* g_RecvBuffer;
};

}

#endif
