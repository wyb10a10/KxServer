/* 
*   Core 服务器核心文件
*   定义跨平台的标准通讯模块，时间，轮询器
*   
*   2013-04-08 By 宝爷
*
*/
#ifndef __ICORE_H__
#define __ICORE_H__

#define MILLION 1000000.0f
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

//消除平台相关的时间，Socket差异
#ifdef WIN32

#include <WinSock2.h>
#include <WinSock.h>
#include <Windows.h>
#include <time.h>

typedef SOCKET COMMUNICATIONID;
typedef int KXSockLen;

#define snprintf  sprintf_s
#define KXINVALID_SOCKET (INVALID_SOCKET)

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval * val, struct timezone * zone);

#pragma comment(lib, "ws2_32.lib")

#else

#define KXINVALID_SOCKET (-1)

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/time.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

typedef int COMMUNICATIONID;
typedef socklen_t KXSockLen;

#endif

//定义时间结构体方便计算和比较
struct TimeVal
{
	inline void SetFromfloat(float t)
	{
		tv_sec = (long)t;
		tv_usec = (long)((t - tv_sec) * MILLION);
	}

	inline float Getfloat()
	{
		return tv_sec + tv_usec/MILLION;
	}

	inline void TimeValAdd(const TimeVal &timeVal)
	{
		tv_sec += timeVal.tv_sec;
		tv_usec += timeVal.tv_usec;
	}

	inline void TimeValSub(const TimeVal &timeVal)
	{
		tv_sec -= timeVal.tv_sec;
		tv_usec -= timeVal.tv_usec;
	}

	inline void TimeValNow()
	{
		gettimeofday((struct timeval *)this,  0);
	}

	//if i bigger return true, else return false
	inline bool Compaire(const TimeVal &timeVal)
	{
		return (tv_sec - timeVal.tv_sec + (tv_usec - timeVal.tv_usec)/MILLION > 0.0f);
	}

	long tv_sec;		// seconds
	long tv_usec;		// microSeconds
};


typedef sockaddr_in SocketAddr;

namespace KxServer {

enum POLL_TYPE
{
	POLLTYPE_UNKNOWN = 0,
	POLLTYPE_IN = 1,
	POLLTYPE_OUT = 4,
	POLLTYPE_ERR = 8
};

//front declared 
class ICommunication;
class ICommunicationPoller;

//逻辑处理模块接口
class IBaseModule
{
public:
    virtual ~IBaseModule()
    {

    }

	//return the length your package need
	virtual int RequestLen(char* buffer, unsigned int len) = 0;

	//when you recv a msg, Process will be call, the target is the msg sender
	virtual void Process(char* buffer, unsigned int len, ICommunication *target) = 0;

    //when socket is error, call this callback
    virtual void ProcessError(ICommunication *target)
    {

    }
};

//通讯对象接口，会有TCP，UDP，UnixSocket，共享内存等实现
//Communication Object Interface, TCP, UDP, Unix Socket, Pipe+Shm
class ICommunication
{
public:
	ICommunication()
		:m_Poller(NULL),
		m_ProcessModule(NULL),
		m_Tag(0)
	{

	}

    virtual ~ICommunication()
    {

    }

	//call by user
	virtual int Send(char* buffer, unsigned int len) = 0;

	//call by framework, if user call this function, must call OnError when error
	virtual int Recv(char* buffer, unsigned int len) = 0;
	
	//get communication id, maybe fd int type int linux or SOCKET type in windows
	virtual COMMUNICATIONID GetCommunicationID() = 0;

	//call by poller, process half pkg, stick pkg
	virtual int OnRecv() = 0;
	
	//call by poller, process send fail, resend
	virtual int OnSend() = 0;
	
	//call by poller
	virtual int OnError() = 0;

    //call by user
    virtual void Close() = 0;

	virtual void SetModule(IBaseModule *model)
	{
		m_ProcessModule = model;
	}

	virtual IBaseModule* GetModule()
	{
		return m_ProcessModule;
	}

	inline void SetTag(int tag)
	{
		m_Tag = tag;
	}

	inline int GetTag()
	{
		return m_Tag;
	}

	inline void SetPoller(ICommunicationPoller* poller)
	{
		m_Poller = poller;
	}

	//get the poll type
	virtual int PollType()
	{
		return m_PollType;
	}

	virtual void SetPollType(int type)
	{
		m_PollType = type;
	}

protected:
	IBaseModule*			m_ProcessModule;
	ICommunicationPoller*	m_Poller;
	int						m_Tag;
	int						m_PollType;
};

//轮询器接口，会有Poll，Select，Epoll，Iocp等实现
//Communication Poller, Select, E-poll(linux), [IOCP(windows)]
class ICommunicationPoller
{
public:
    virtual ~ICommunicationPoller()
    {

    }

	//poll all communication object, and process event
	virtual int Poll() = 0;

	virtual int AddPollObject(ICommunication* obj, int type) = 0;
	
	virtual int ModifyPollObject(ICommunication* obj, int type) = 0;
	
	virtual int RemovePollObject(ICommunication* obj) = 0;
};

}

#endif
