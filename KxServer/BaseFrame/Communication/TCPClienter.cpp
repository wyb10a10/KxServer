#include <iostream>
#include "TCPClienter.h"
#include "CommPool.h"
#include "MemPool.h"

using namespace std;

//1024 * 6
#define BUFF_SIZE 6144
#define MAX_PKGLEN 1<<16

namespace KxServer {

char* CTCPClienter::g_RecvBuffer = NULL;

CTCPClienter::CTCPClienter(CBaseSocket* sock, ICommunicationPoller* poller)
					:m_Socket(sock),
					m_SendBuffer(NULL),
					m_RecvBuffer(NULL),
					m_SendBufferLen(0),
					m_RecvBufferLen(0),
					m_SendBufferOffset(0),
                    m_RecvBufferOffset(0)
{
    //分配全局的接收缓冲区
    if (NULL == g_RecvBuffer)
    {
        g_RecvBuffer = (char*)CMemManager::GetInstance()->MemAlocate(BUFF_SIZE);
    }

	m_Socket->SocketNonBlock(true);
	CCommPool::GetInstance()->AddCommuncation(this);
	m_PollType = POLLTYPE_UNKNOWN;

	if (NULL != poller)
	{
		m_Poller = poller;
		m_Poller->AddPollObject(this, POLLTYPE_IN);
	}
}

CTCPClienter::~CTCPClienter(void)
{
	delete m_Socket;

    if (NULL != m_SendBuffer)
    {
        CMemManager::GetInstance()->MemRecycle(m_SendBuffer, m_SendBufferLen);
    }

    if (NULL != m_RecvBuffer)
    {
        CMemManager::GetInstance()->MemRecycle(m_RecvBuffer, m_SendBufferLen);
    }

	//clear list and data in list
	BufferNode* node = m_BufferList.Next();
    BufferNode* nodeNext = NULL;
    while(NULL != node)
	{
        CMemManager::GetInstance()->MemRecycle(node->buffer, node->len);
        nodeNext = m_BufferList.Next();
		//delete[] node->buffer;
		delete node;
        node = nodeNext;
	}
}

//call by user
int CTCPClienter::Send(char* buffer, unsigned int len)
{
	int ret = 0;
	//no buffer need to send before, or call by OnSend
	if (NULL == m_SendBuffer 
		|| buffer == (m_SendBuffer + m_SendBufferOffset))
	{
		ret = m_Socket->SocketSend(buffer, len);
	}
	
	if(ret < 0)
	{
		//if no eagain or ewouldblock
        if(m_Socket->IsSocketError() 
            && !(m_PollType & POLLTYPE_IN))
		{
            if(NULL != m_Poller)
            {
                m_Poller->RemovePollObject(this);
            }

			//socket invalid remove from poll, destroy this
			OnError();
			return ret;
		}
		else
		{
			//you need send all buffer again
			ret = 0;		
		}
	}
	
	if (ret < (int)len)
	{
		//save to buffer and add pollout
		if (NULL != m_Poller 
			&& ( NULL == m_SendBuffer || buffer != (m_SendBuffer + m_SendBufferOffset)))
		{
			len -= ret; 
			
            //char* buf = new char[len];
            char* buf = (char*)MemMgrAlocate(len);
            memcpy(buf, buffer + ret, len);
			m_BufferList.PushBack(buf, len);

			m_Poller->ModifyPollObject(this, m_PollType | POLLTYPE_OUT);
		}
	}

	return ret;
}

//call by framework
int CTCPClienter::Recv(char* buffer, unsigned int len)
{
	int ret = m_Socket->SocketRecv(buffer, len);

	//when ret = 0, socket has been close
	if (ret <= 0)
	{
		if (m_Socket->IsSocketError())
		{
			//close
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}

	return ret;
}

//get communication id, maybe fd int type int linux or SOCKET type in windows
COMMUNICATIONID CTCPClienter::GetCommunicationID()
{
	return m_Socket->GetSocket();
}

//call by poller
int CTCPClienter::OnRecv()
{
	int requestLen = 0;
	int ret = Recv(g_RecvBuffer, BUFF_SIZE);

	//if ret < 0 
	if (NULL != m_ProcessModule && ret >= 0)
	{
		char* processBuf = g_RecvBuffer;
		char* stickBuf = NULL;

        //如果有半包，拼接到半包的后面，注意newsize
		if (NULL != m_RecvBuffer)
		{
			//append to my recvbuffer
			unsigned int newsize = ret;
			if ((m_RecvBufferLen - m_RecvBufferOffset) < static_cast<unsigned int>(ret))
			{
                //ret is larger then m_RecvBufferLen - m_RecvBufferOffset, no stick Buf
				newsize = m_RecvBufferLen - m_RecvBufferOffset;
				stickBuf = processBuf + newsize;
			}

            //copy to my buffer
            memcpy(m_RecvBuffer + m_RecvBufferOffset, processBuf, newsize);
            //ret all buffer length
            ret += m_RecvBufferOffset;
            m_RecvBufferOffset += newsize;
            processBuf = m_RecvBuffer;
		}

		requestLen = m_ProcessModule->RequestLen(processBuf, ret);
		if (requestLen <= 0 || requestLen > MAX_PKGLEN)
		{
			//package data error, close socket
			return requestLen;
		}
		
		if (ret < requestLen)
		{
			//copy to recv buffer
			if (NULL == m_RecvBuffer)
			{
                m_RecvBuffer = (char*)MemMgrAlocate(requestLen);
				m_RecvBufferLen = requestLen;
				m_RecvBufferOffset = ret;
				memcpy(m_RecvBuffer, processBuf, ret);
			}

			//has been append
			return ret;
		}

		while (ret >= requestLen)
		{
			m_ProcessModule->Process(processBuf, requestLen, this);
			processBuf += requestLen;

			if (NULL != m_RecvBuffer)
			{
				processBuf = stickBuf;
                MemMgrRecycle(m_RecvBuffer, m_RecvBufferLen);

                m_RecvBuffer = NULL;
				m_RecvBufferOffset = m_RecvBufferLen = 0;
			}

			ret -= requestLen;
			if (ret > 0
                && NULL != processBuf)
			{
				//next 
				requestLen = m_ProcessModule->RequestLen(processBuf, ret);
                if (requestLen <= 0 || requestLen > MAX_PKGLEN)
                {
                    return requestLen;
                }
                //半包缓存
                else if (ret < requestLen)
                {
                    //m_RecvBuffer must be NULL
                    m_RecvBuffer = (char*)MemMgrAlocate(requestLen);
                    m_RecvBufferLen = requestLen;
                    m_RecvBufferOffset = ret;
                    memcpy(m_RecvBuffer, processBuf, ret);
                    return ret;
                }
			}
		}
	}

	return ret;
}

//call by poller
int CTCPClienter::OnSend()
{
again:
	if (NULL == m_SendBuffer)
	{
		BufferNode* node = m_BufferList.Next();
		if (NULL != node)
		{
			m_SendBuffer = node->buffer;
			m_SendBufferLen = node->len;
			m_SendBufferOffset = 0;
			delete node;
		}
		else
		{
			//nothing need to send
			return 0;
		}
	}

	int len = Send(m_SendBuffer, m_SendBufferLen);
	
	//send finish
	if(len >= (int)(m_SendBufferLen - m_SendBufferOffset))
	{
		//delete [] m_SendBuffer;
        //回收已经发送出去的内存
        MemMgrRecycle(m_SendBuffer, m_SendBufferLen);

        BufferNode* node = m_BufferList.Head();
		m_SendBuffer = NULL;
		m_SendBufferLen = m_SendBufferOffset = 0;
		
		if (NULL == node)
		{
			//don't send again
			m_Poller->ModifyPollObject(this, POLLTYPE_IN);
		}
		//send next
		else
		{
			goto again;
		}
	}
	//send half, try again
	//len < m_SendBufferLen - m_SendBufferOffset && len >= 0
	else if(len < (int)(m_SendBufferLen - m_SendBufferOffset) && len >= 0)
	{
		//Send Again
		m_SendBufferOffset += len;
		m_Poller->ModifyPollObject(this, m_PollType | POLLTYPE_OUT);
	}

	return len;
}

//call by poller
int CTCPClienter::OnError()
{
    if (NULL != m_ProcessModule)
    {
        m_ProcessModule->ProcessError(this);
    }

    //socket is invalid must be close
    CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());

    return 0;
}

void CTCPClienter::Close()
{
    if (NULL != m_Poller)
    {
        m_Poller->RemovePollObject(this);
    }

    CCommPool::GetInstance()->RemoveCommuncation(GetCommunicationID());
}

}
