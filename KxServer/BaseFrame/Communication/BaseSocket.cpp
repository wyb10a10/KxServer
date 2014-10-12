#include "BaseSocket.h"

namespace KxServer {

CBaseSocket::CBaseSocket(int socketType)
{
#ifdef WIN32

    WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		return;
	}

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup();
		return; 
	}

#endif

	m_SocketType = socketType;
	m_SocketState = SOCKET_STATEUNINIT;
	m_bNonBlock = false;
}

CBaseSocket::CBaseSocket(int socketType, COMMUNICATIONID handle)
{
	m_Socket = handle;
	m_SocketType = socketType;
	//ths socket is connected
	m_SocketState = SOCKET_STATECONNECTED;
}

CBaseSocket::~CBaseSocket(void)
{
	SocketClose();
}

// init Socket, warn!!! you must call this function when you new the Socket, except new by Accept
int CBaseSocket::SocketInit()
{
	if (SOCKET_STATEUNINIT != m_SocketState)
	{
		return SOCKET_ERRORSTATE;
	}

	if(SOCKET_TYPETCP == m_SocketType)
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	}
	else if (SOCKET_TYPEUDP == m_SocketType)
	{
		m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		return SOCKET_ERRORTYPE;
	}

	bool isAddrReuse = true;
	//setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR,
	// (const char*)&isAddrReuse, sizeof(isAddrReuse));
	m_SocketState = SOCKET_STATEINIT;
	return SOCKET_SUCCESS;
}

// listen, just TCP type Socket can call this function
int CBaseSocket::SocketListen(int maxListenQueue)
{
	if(SOCKET_STATEBIND != m_SocketState)
	{
		return SOCKET_ERRORSTATE;
	}

	if (SOCKET_TYPETCP != m_SocketType)
	{
		return SOCKET_ERRORTYPE;
	}

	m_SocketState = SOCKET_STATELISTENING;
	return listen(m_Socket, maxListenQueue);
}

// TCP connect by addr and prot
int CBaseSocket::SocketConnect(const char* addr, int port)
{
	if(SOCKET_STATEINIT != m_SocketState)
	{
		return SOCKET_ERRORSTATE;
	}

	if (SOCKET_TYPETCP != m_SocketType)
	{
		return SOCKET_ERRORTYPE;
	}

	sockaddr_in name;
	SocketInitAddr(name, port, addr);
	m_SocketState = SOCKET_STATECONNECTED;
    int ret = connect(m_Socket, (sockaddr*)&name, sizeof(sockaddr));
	if(ret < 0 && IsSocketError())
    {
        m_SocketState = SOCKET_ERRORSTATE;
    }

    return ret;
}


// bind to a ip and port, addr can be NULL, it will bind to every network card
int CBaseSocket::SocketBind(const char* addr, int port)
{
	if(SOCKET_STATEINIT != m_SocketState)
	{
		return SOCKET_ERRORSTATE;
	}

	sockaddr_in name;
	SocketInitAddr(name, port, addr);	
	m_SocketState = SOCKET_STATEBIND;
	return bind(m_Socket, (sockaddr*)&name, sizeof(sockaddr));
}


// TCP Accept return a Socket client object, you need delete it by yourself
CBaseSocket* CBaseSocket::SocketAccept(void)
{
	if(SOCKET_STATELISTENING != m_SocketState)
	{
		return NULL;
	}

	if (SOCKET_TYPETCP != m_SocketType)
	{
		return NULL;
	}

	sockaddr_in name;
	int len = sizeof(sockaddr);
	COMMUNICATIONID handle = accept(m_Socket, (sockaddr*)&name, (KXSockLen*)&len);
	//accept success 
	if (handle != KXINVALID_SOCKET)
	{
		return new CBaseSocket(m_SocketType, handle);
	}
	
	return NULL;
}


// Send Data
int CBaseSocket::SocketSend(const char* buffer, int size)
{
	if (SOCKET_TYPETCP == m_SocketType)
	{
		return send(m_Socket, buffer, size, 0);
	}
	else if(SOCKET_TYPEUDP == m_SocketType)
	{
		return sendto(m_Socket, buffer, size, 0, (sockaddr*)&m_SockAddr, sizeof(m_SockAddr));
	}

	return SOCKET_ERRORTYPE;
}


// Recv Data
int CBaseSocket::SocketRecv(char* buffer, int size)
{
	if (SOCKET_TYPETCP == m_SocketType)
	{
		return recv(m_Socket, buffer, size, 0);
	}
	else if(SOCKET_TYPEUDP == m_SocketType)
	{
		int len = sizeof(sockaddr);
		return recvfrom(m_Socket, buffer, size, 0, (sockaddr*)&m_SockAddr, (KXSockLen*)&len);
	}

	return SOCKET_ERRORTYPE;
}


// Close the socket
void CBaseSocket::SocketClose(void)
{
	m_SocketState = SOCKET_STATEUNINIT;
#ifdef WIN32
    closesocket(m_Socket);
#else
	close(m_Socket);
#endif
}

void CBaseSocket::SocketSetAddr(SocketAddr &name)
{
	if(SOCKET_TYPETCP == m_SocketType)
	{
		return;
	}

	m_SockAddr = name;
}

void CBaseSocket::SocketSetAddr(const char* ip, int port)
{
	if (SOCKET_TYPETCP == m_SocketType)
	{
		return;
	}

	SocketInitAddr(m_SockAddr, port, ip);
}

bool CBaseSocket::IsSocketError()
{
#ifdef WIN32
	return (WSAEWOULDBLOCK != WSAGetLastError());
#else
	return (errno != EWOULDBLOCK && errno != EAGAIN);
#endif
}

void CBaseSocket::SocketNonBlock(bool bNonBlock)
{
#ifdef WIN32
	// when bNonBlock is true, nonblock is 1
	// when bNonBlock is false, nonblock is 0
	ULONG nonblock = bNonBlock;
	ioctlsocket(m_Socket, FIONBIO, &nonblock);
#else
	int flags = fcntl(m_Socket, F_GETFL, 0);
	bNonBlock ? flags |= O_NONBLOCK : flags -= O_NONBLOCK;
	fcntl(m_Socket, F_SETFL, flags);
#endif
}

void CBaseSocket::SocketNonDelay()
{
//#ifdef WIN32
	const char opt = 1;
	setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(char));
//#else	
//#endif
}

void CBaseSocket::SocketInitAddr(SocketAddr &name, int port, const char* ip /* = NULL */)
{
	name.sin_family		= AF_INET;
	name.sin_port		= htons(port);

#ifdef WIN32
	if (NULL == ip)
	{
		name.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	}
	else
	{
		name.sin_addr.S_un.S_addr	= inet_addr(ip);
	}
#else
	if (NULL == ip)
	{
		name.sin_addr.s_addr		= htonl(INADDR_ANY);
	}
	else
	{
		name.sin_addr.s_addr		= inet_addr(ip);
	}
#endif

}

}
