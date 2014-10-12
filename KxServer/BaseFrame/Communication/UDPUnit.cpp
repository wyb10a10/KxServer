#include "UDPUnit.h"
#include "CommPool.h"
#include "MemPool.h"

using namespace KxServer;

#define BUFF_SIZE       2000

namespace KxServer {

CUDPUnit::CUDPUnit(ICommunicationPoller* poller)
{
	m_Socket = new CBaseSocket(SOCKET_TYPEUDP);
	m_Socket->SocketInit();
	m_Socket->SocketNonBlock(true);
	m_PollType = POLLTYPE_UNKNOWN;

	if (NULL != poller)
	{
		m_Poller = poller;
		m_Poller->AddPollObject(this, POLLTYPE_IN);
	}

    //从内存池中取出内存
    m_RecvBuffer = (char*)CMemManager::GetInstance()->MemAlocate(BUFF_SIZE);

	CCommPool::GetInstance()->AddCommuncation(this);
}

CUDPUnit::~CUDPUnit(void)
{
    CMemManager::GetInstance()->MemRecycle(m_RecvBuffer, BUFF_SIZE);

	delete m_Socket;
}

//call by user
int CUDPUnit::Send(char* buffer, unsigned int len)
{
	int ret = m_Socket->SocketSend(buffer, len);
	if (ret < 0 && m_Socket->IsSocketError())
	{
        if (NULL != m_Poller)
        {
            m_Poller->RemovePollObject(this);
        }

		//CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());
	    OnError();
    }
	return ret;
}

//call by framework
int CUDPUnit::Recv(char* buffer, unsigned int len)
{
	int ret = m_Socket->SocketRecv(buffer, len);
	if (ret < 0 && !m_Socket->IsSocketError())
	{
        //发生错误但Socket可用
        return 0;

		//if (NULL != m_Poller)
		//{
		//	m_Poller->RemovePollObject(this);
		//}
		//CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());
	    //OnError();
    }
	return ret;
}

//get communication id, maybe fd int type int linux or SOCKET type in windows
COMMUNICATIONID CUDPUnit::GetCommunicationID()
{
	return m_Socket->GetSocket();
}

//call by poller
int CUDPUnit::OnRecv()
{
	int ret = Recv(m_RecvBuffer, BUFF_SIZE);
	
    if (NULL != m_ProcessModule && ret > 0)
	{
		m_ProcessModule->Process(m_RecvBuffer, ret, this);
	}

	return ret;
}

//call by poller
int CUDPUnit::OnSend()
{
	//nothing to send
	return 0;
}

//call by poller
int CUDPUnit::OnError()
{
    if (NULL != m_ProcessModule)
    {
        m_ProcessModule->ProcessError(this);
    }

    //socket is invalid must be close
    CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());

	return 0;
}

void CUDPUnit::Close()
{
    if (NULL != m_Poller)
    {
        m_Poller->RemovePollObject(this);
    }

    CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());
}

int CUDPUnit::Bind(char* ip, int port)
{
	return m_Socket->SocketBind(ip, port);
}

int CUDPUnit::SetSendToAddr(char* ip, int port)
{
	m_Socket->SocketSetAddr(ip, port);
	return 0;
}

}
