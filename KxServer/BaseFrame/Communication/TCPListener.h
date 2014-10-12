/*
 * TCP Socket 监听者
 * Client Module 用于给客户端配置对应的处理模块
 * Process Module 用于对客户端的连接进行处理，例如防火墙验证，IP过滤等等
 *
 *  2013-04-15 By 宝爷
 *  
 */
#ifndef __TCPLISTENER_H__
#define __TCPLISTENER_H__

#include "ICore.h"
#include "BaseSocket.h"

namespace KxServer {

class CTCPListener :
	public ICommunication
{
public:
	CTCPListener(int port, char* addr = NULL);
	virtual ~CTCPListener(void);

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

	inline void SetClientModule(IBaseModule* module)
	{
		m_ClientModule = module;
	}

	inline IBaseModule* GetClientModule()
	{
		return m_ClientModule;
	}

protected:

	CBaseSocket* m_Socket;
	IBaseModule* m_ClientModule;
};

}

#endif
