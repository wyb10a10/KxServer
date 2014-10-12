/*
 * 服务端监听Socket Accept之后返回的Socket对象
 * 通过此对象可以与客户端通讯
 * 
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __TCPCLIENTER_H__
#define __TCPCLIENTER_H__

#include "ICore.h"
#include "BaseSocket.h"
#include "BufferList.h"

namespace KxServer {

//solve half pkg, stick pkg and faile in recv, and send faile
class CTCPClienter : 
	public ICommunication 
{
public:
	CTCPClienter(CBaseSocket* sock, ICommunicationPoller* poller);
	virtual ~CTCPClienter(void);

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

protected:

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
