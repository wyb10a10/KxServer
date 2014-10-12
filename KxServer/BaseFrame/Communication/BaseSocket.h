/*
 * 基础Socket通讯对象
 * 定义了非阻塞，非延迟，发送和接收接口
 * 定义Socket错误异常
 *
 *  2013-04-12 By 宝爷
 *  
 */
#ifndef __KX_SOCKET__
#define __KX_SOCKET__

#include "ICore.h"

namespace KxServer {

enum SOCKET_TYPE
{
	SOCKET_TYPETCP,
	SOCKET_TYPEUDP
};

enum SOCKET_STATE
{
	SOCKET_STATEUNINIT,					//未初始化
	SOCKET_STATEINIT,					//已初始化
	SOCKET_STATEBIND,					//已绑定
	SOCKET_STATECONNECTED,				//已连接
	SOCKET_STATELISTENING,				//正在监听
	SOCKET_STATEERROR					//错误
};

enum SOCKET_CODE
{
	SOCKET_ERRORUNKNOW	= -9999,		//未知的Socket错误
	SOCKET_ERRORFAILE,					//Socket操作失败
	SOCKET_ERRORTYPE,					//Socket类型错误:非TCP和UDP，或UDP调用TCP操作
	SOCKET_ERRORSTATE,					//Socket状态错误,例如未初始化调用connect，或未listen调用accept
	SOCKET_ERRORAGAIN,					//当设置为非阻塞套接字时，send或recv数据不完整，需再次send或recv

	SOCKET_SUCCESS		= 0				//成功返回
};

class CBaseSocket
{
public:
	CBaseSocket(int socketType = SOCKET_TYPETCP);

	//传入一个已经连接上的句柄
	CBaseSocket(int socketType, COMMUNICATIONID handle);
	
	virtual ~CBaseSocket(void);
	
	// 初始化Socket
	int SocketInit();
	
	// 监听，TCPSocket类型下可以调用
	int SocketListen(int maxListenQueue);
	
	// 链接指定的IP和端口
	int SocketConnect(const char* addr, int port);
	
	// 绑定到指定的Ip和端口
	int SocketBind(const char* addr, int port);
	
	// Accept返回一个Socket连接对象
	CBaseSocket* SocketAccept(void);
	
	// 关闭Socket连接
	void SocketClose(void);

	
	// 往套接口写数据
	int SocketSend(const char* buffer, int size);
	
	// 接收数据
	int SocketRecv(char* buffer, int size);

	//设置当前的SocketAddr,这个SocketAddr将会被用于UDP发送
	void SocketSetAddr(SocketAddr &name);
	
	//设置当前的SocketAddr,这个SocketAddr将会被用于UDP发送
	void SocketSetAddr(const char* ip, int port);

	//根据平台判断是否EAGAIN错误
	bool IsSocketError();

	void SocketNonBlock(bool bNonBlock);

	inline bool SocketNonBlock()
	{
		return m_bNonBlock;
	}

	//不使用Ngale算法延时发送
	void SocketNonDelay();
	
	//获取当前的Socket地址,这个地址在每次UDP接收以及SetAddr时会被更新
	inline SocketAddr SocketGetAddr()
	{
		return m_SockAddr;
	}

	//Get The Socket Handle
	inline COMMUNICATIONID GetSocket()
	{
		return m_Socket;
	}

private:

	//初始化Addr结构体，包含了不同平台的实现
	void SocketInitAddr(SocketAddr &name, int port, const char* ip = NULL);

private:
	COMMUNICATIONID	m_Socket;
	int				m_SocketType;
	int				m_SocketState;
	SocketAddr	    m_SockAddr;
	bool			m_bNonBlock;
};

}

#endif
