#ifndef __REF_H__
#define __REF_H__

class CRef
{
protected:
	CRef(void);

public:
	virtual ~CRef(void);

	void retain();

	void release();

	unsigned int getReferenceCount() const;

protected:
	unsigned int m_ReferenceCount;
};

#endif
