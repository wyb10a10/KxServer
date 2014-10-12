#include "Ref.h"

CRef::CRef(void)
	:m_ReferenceCount(1)
{
}

CRef::~CRef(void)
{
}

void CRef::retain()
{
	++m_ReferenceCount;
}

void CRef::release()
{
	--m_ReferenceCount;

	if (m_ReferenceCount == 0)
	{
		delete this;
	}
}

unsigned int CRef::getReferenceCount() const
{
	return m_ReferenceCount;
}
