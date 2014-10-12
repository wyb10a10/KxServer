#include "TCPListener.h"
#include "TCPClienter.h"
#include "CommPool.h"

namespace KxServer {

CTCPListener::CTCPListener(int port, char* addr)
{
	m_ProcessModule = NULL;
	m_PollType = POLLTYPE_UNKNOWN;
	m_Socket = new CBaseSocket(SOCKET_TYPETCP);
	m_Socket->SocketInit();
	m_Socket->SocketBind(addr, port);
	m_Socket->SocketListen(100);
	m_Socket->SocketNonBlock(true);
	CCommPool::GetInstance()->AddCommuncation(this);
}

CTCPListener::~CTCPListener(void)
{
	delete m_Socket;
}

//call by user
int CTCPListener::Send(char* buffer, unsigned int len)
{
	return 0;
}

//call by framework
int CTCPListener::Recv(char* buffer, unsigned int len)
{
	return 0;
}

//get communication id, maybe fd int type int linux or SOCKET type in windows
COMMUNICATIONID CTCPListener::GetCommunicationID()
{
	return m_Socket->GetSocket();
}

//call by poller
int CTCPListener::OnRecv()
{
	CBaseSocket* client = NULL;
	do 
	{
		client = m_Socket->SocketAccept();
		if (client)
		{
			CTCPClienter *tcpc = new CTCPClienter(client, m_Poller);
			if (NULL != tcpc)
			{
                tcpc->SetModule(m_ClientModule);
                
                if(NULL != m_ProcessModule)
                {
                    //May Be Close
                    m_ProcessModule->Process(NULL, 0, tcpc);
                }
            }
		}
	} while (client != NULL);

	return 0;
}

//call by poller
int CTCPListener::OnSend()
{
	return 0;
}

//call by poller
int CTCPListener::OnError()
{
    if (NULL != m_ProcessModule)
    {
        m_ProcessModule->ProcessError(this);
    }

    //socket is invalid must be close
    Close();

    return 0;
}

void CTCPListener::Close()
{
    if (NULL != m_Poller)
    {
        m_Poller->RemovePollObject(this);
    }

	CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());
}

}
