/*
 * BufferList 缓冲区列表
 * 用于缓存数据块
 *
 *  2013-04-20 By 宝爷
 *  
 */
#ifndef __BUFFERLIST_H__
#define __BUFFERLIST_H__

#include <stdlib.h>

namespace KxServer {

struct BufferNode
{
	BufferNode(char* buf, unsigned int l)
		:buffer(buf),
		len(l),
		next(NULL)
	{
	}

	BufferNode()
		:buffer(NULL),
		len(0),
		next(NULL)
	{
	}

	char* buffer;
	unsigned int len;
	BufferNode* next;
};

class CBufferList
{
public:
	CBufferList(void);
	virtual ~CBufferList(void);

	//pop next node from head
	BufferNode* Next()
	{
		BufferNode* node = m_Head;
		if (m_Tail == m_Head)
		{
			//this is the last one
			m_Head = m_Tail = NULL;
		}
		else
		{
			//set to next one
			m_Head = m_Head->next;
		}

		return node;
	}

	void PushBack(char* buffer, unsigned int len)
	{
		BufferNode* node = new BufferNode(buffer, len);
		PushBack(node);
	}

	//add node to tail
	void PushBack(BufferNode* node)
	{
		if (NULL == m_Tail)
		{
			//add first node
			m_Tail = m_Head = node;
		}
		else
		{
			m_Tail->next = node;
			m_Tail = node;
		}
	}

	//get list head
	BufferNode* Head()
	{
		return m_Head;
	}

	//delete all node
	void Clear();

private:
	BufferNode* m_Head;
	BufferNode* m_Tail;
};

}

#endif
