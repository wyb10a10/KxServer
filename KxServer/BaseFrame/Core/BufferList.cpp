#include "BufferList.h"

namespace KxServer {

CBufferList::CBufferList(void)
:m_Head(NULL),
m_Tail(NULL)
{
}

CBufferList::~CBufferList(void)
{
}

void CBufferList::Clear()
{
	while(NULL != m_Head)
	{
		BufferNode* node = m_Head;
		m_Head = m_Head->next;
		delete node;
	}
}

}
