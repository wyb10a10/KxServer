/*
 * 实现UDP协议Socket传输单元
 *
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __UDPUNIT_H__
#define __UDPUNIT_H__

#include "ICore.h"
#include "BaseSocket.h"

namespace KxServer {

class CUDPUnit
	:public ICommunication
{
public:
	CUDPUnit(ICommunicationPoller* poller);
	virtual ~CUDPUnit(void);

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

	int Bind(char* ip, int port);

	int SetSendToAddr(char* ip, int port);

private:
	CBaseSocket*    m_Socket;
    char*           m_RecvBuffer;
};

}

#endif
