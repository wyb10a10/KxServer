#include "TimerManager.h"

using namespace std;

#ifdef WIN32

int gettimeofday(struct timeval * val, struct timezone * zone)
{
	if (val)
	{
		LARGE_INTEGER liTime, liFreq;
		QueryPerformanceFrequency( &liFreq );
		QueryPerformanceCounter( &liTime );
		val->tv_sec     = (long)( liTime.QuadPart / liFreq.QuadPart );
		val->tv_usec    = (long)( liTime.QuadPart * 1000000.0 / liFreq.QuadPart - val->tv_sec * 1000000.0 );
	}

	return 0;
}

#endif

namespace KxServer {

ITimerObject::ITimerObject()
    :m_TimerList(NULL),
    m_Prev(NULL),
    m_Next(NULL),
    m_TimeVal(),
    m_fTimeVal(0.0f),
    m_bIsAlive(false),
    m_bIsAutoRelease(false)
{

}

ITimerObject::~ITimerObject()
{

}

void ITimerObject::RemoveSelf()
{
    if (NULL != m_TimerList)
    {
        m_TimerList->Remove(this);
        m_bIsAlive = false;
    }
}

IRepeatTimeObject::IRepeatTimeObject()
	:m_RepeatTimes(0),
	m_Delta(0.0f)
{

}

void IRepeatTimeObject::OnTimer(const TimeVal& now)
{
	--m_RepeatTimes;
	if (0 < m_RepeatTimes)
	{
		m_bIsAlive = true;
		SetTimeOut(GetTimeOut() + m_Delta);
	}
}

CTimerList::CTimerList()
    :m_Length(0),
	m_Type(TimerListAgile),
    m_Head(NULL),
    m_Tail(NULL),
	m_Timer(NULL)
{

}

CTimerList::~CTimerList()
{

}

void CTimerList::PushBack(ITimerObject* obj)
{
    if (NULL == obj
        || NULL != obj->m_TimerList)
    {
        return;
    }

    if (NULL == m_Tail)
    {
        m_Head = obj;
        obj->m_Prev = obj->m_Next = NULL;

    }
    else
    {
        m_Tail->m_Next = obj;
        obj->m_Prev = m_Tail;
        obj->m_Next = NULL;
    }
    
    m_Tail = obj;
    obj->m_TimerList = this;
	obj->retain();
    ++m_Length;
}

void CTimerList::PushFront(ITimerObject* obj)
{
    if (NULL == obj
        || NULL != obj->m_TimerList)
    {
        return;
    }

    if (NULL == m_Head)
    {
        m_Tail = obj;
        obj->m_Prev = obj->m_Next = NULL;
    }
    else
    {
        m_Head->m_Prev = obj;
        obj->m_Next = m_Head;
        obj->m_Prev = NULL;
    }

    m_Head = obj;
    obj->m_TimerList = this;
	obj->retain();
    ++m_Length;
}

void CTimerList::Remove(ITimerObject* obj)
{
    if (NULL == obj
        || obj->m_TimerList != this)
    {
        return;
    }

    if (m_Head == obj)
    {
        m_Head = obj->m_Next;
        if (NULL != m_Head)
        {
            m_Head->m_Prev = NULL;
        }
        else
        {
            //head is the tail
            m_Tail = NULL;
        }
    }
    else if (m_Tail == obj)
    {
        //is tail and not the head
        m_Tail = obj->m_Prev;
        m_Tail->m_Next = NULL;
    }
    else
    {
        //not head and tail
        obj->m_Prev->m_Next = obj->m_Next;
        obj->m_Next->m_Prev = obj->m_Prev;
    }

    obj->m_Next = obj->m_Prev = NULL;
    obj->m_TimerList = NULL;
	obj->release();
    --m_Length;
}

void CTimerList::Insert(ITimerObject* obj, float timeOut)
{
    if (NULL == obj
        || NULL != obj->m_TimerList)
    {
        return;
    }

    if (NULL == m_Head)
    {
        PushFront(obj);
    }
    else
    {
        //search from head
        ITimerObject* pNode = m_Head;
        while(NULL != pNode)
        {
            //timeOut is large then pNode->timeOut
            if (pNode->GetTimeOut() > timeOut)
            {
                //insert before pNode
                if (m_Head == pNode)
                {
                    //obj to be head
                    obj->m_Prev = NULL;
                    obj->m_Next = m_Head;
                    m_Head->m_Prev = obj;
                    m_Head = obj;
                }
                else
                {
                    obj->m_Next = pNode;
                    obj->m_Prev = pNode->m_Prev;
                    obj->m_Prev->m_Next = obj;
                    pNode->m_Prev = obj;
                }

				obj->retain();
                obj->m_TimerList = this;
                ++m_Length;

                break;
            }
            else if (m_Tail == pNode)
            {
                PushBack(obj);
                break;
            }
            else
            {
				//move to next
                pNode = pNode->m_Next;
            }
        }
    }
}

void CTimerList::RInsert(ITimerObject* obj, float timeOut)
{
    if (NULL == obj
        || NULL != obj->m_TimerList)
    {
        return;
    }

    if (NULL == m_Tail)
    {
        PushBack(obj);
    }
    else
    {
        ITimerObject* pNode = m_Tail;
        while(NULL != pNode)
        {
            //timeOut is large then pNode->timeOut
            if (pNode->GetTimeOut() < timeOut)
            {
                //insert after pNode
                if (m_Tail == pNode)
                {
                    obj->m_Next = NULL;
                    obj->m_Prev = m_Tail;
                    m_Tail->m_Next = obj;
                    m_Tail = obj;
                }
                else
                {
                    obj->m_Prev = pNode;
                    obj->m_Next = pNode->m_Next;
                    obj->m_Next->m_Prev = obj;
                    pNode->m_Next = obj;
                }

				obj->retain();
                obj->m_TimerList = this;
                ++m_Length;

                break;
            }
            else if (m_Head == pNode)
            {
                PushFront(obj);
                break;
            }
            else
            {
				//move to prev
                pNode = pNode->m_Prev;
            }
        }
    }
}

void CTimerList::Update(float fnow, const TimeVal& now)
{
    while(NULL != m_Head)
    {
        //can add 
        if (m_Head->IsTimeOut(fnow))
        {
            m_Timer = m_Head;
            m_Head->OnTimer(now);
            if (!m_Timer->IsAlive())
            {
                Remove(m_Timer);
				m_Timer = NULL;
            }
			else
			{
				m_Timer->retain();
				Remove(m_Timer);
				if (m_Type == TimerListAgile)
				{
					RInsert(m_Timer, m_Timer->GetTimeOut());
				}
				else
				{
					PushBack(m_Timer);
				}
				m_Timer->release();
				m_Timer = NULL;
			}
        }
        else
        {
            break;
        }
    }
}

CTimerManager* CTimerManager::m_Instance = NULL;

CTimerManager::CTimerManager(void)
{
	gettimeofday((struct timeval*)&(m_Now), 0);
	m_fNow = m_Now.Getfloat();
	m_AglieMidValue = 1.0f;

    m_AglieTimerList = new CTimerList();
}

CTimerManager::~CTimerManager(void)
{
	for (map<int, CTimerList*>::iterator iter = m_FixTimerMap.begin();
		iter != m_FixTimerMap.end(); ++iter)
	{
		delete iter->second;
	}

    delete m_AglieTimerList;
    m_AglieTimerList = NULL;
}

//call this function in main loop
void CTimerManager::UpdateTimer()
{
	gettimeofday((struct timeval*)&(m_Now), 0);
	m_fNow = m_Now.Getfloat();

    //update the fix fast timer map
#ifdef WIN32

	for (map<int, CTimerList*>::iterator iter = m_FixTimerMap.begin();
		iter != m_FixTimerMap.end(); )
	{
        iter->second->Update(m_fNow, m_Now);
        if (0 == iter->second->Length())
		{
			delete iter->second;
			iter = m_FixTimerMap.erase(iter);
		}
		else
		{
			++iter;	
		}
	}

#else

	for (map<int, CTimerList*>::iterator iter = m_FixTimerMap.begin();
		iter != m_FixTimerMap.end(); ++iter)
	{
		iter->second->Update(m_fNow, m_Now);
		if (0 == iter->second->Length())
		{
			delete iter->second;
			m_FixTimerMap.erase(iter);
		}
	}

#endif

    m_AglieTimerList->Update(m_fNow, m_Now);
}

// if you have many timerobject with the same delay time
// or you wan't to repeat this timerobject many times this function will be faster
void CTimerManager::AttachTimerWithFixTime(ITimerObject* obj)
{
    if (NULL == obj)
    {
        return;
    }

    return AttachTimerWithFixTime(obj->GetTimeOut(), obj);
}

void CTimerManager::AttachTimerWithFixTime(float t, ITimerObject* obj)
{
	if (NULL == obj)
	{
		return;
	}

	int key = static_cast<int>(t * MILLION);
	CTimerList* l = GetTimeList(key);

	if (NULL != l)
	{
		t += m_fNow;
		obj->SetTimeOut(t);
        l->PushBack(obj);
	}
}

void CTimerManager::AttachTimerWithFixTime(TimeVal t, ITimerObject* obj)
{
	if (NULL == obj)
	{
		return;
	}

    int key = static_cast<int>(t.Getfloat() * MILLION);
    CTimerList* l = GetTimeList(key);

	if (NULL != l)
	{
		t.TimeValAdd(m_Now);
        obj->SetTimeOut(t);
        l->PushBack(obj);
	}
}

//if you just wan't update once, this function will be better
void CTimerManager::AttachTimerWithAgileTime(ITimerObject* obj)
{
    if (NULL == obj)
    {
        return;
    }

    return AttachTimerWithAgileTime(obj->GetTimeOut(), obj);
}

void CTimerManager::AttachTimerWithAgileTime(float t, ITimerObject* obj)
{
	TimeVal timeVal;
	timeVal.SetFromfloat(t);
	timeVal.TimeValAdd(m_Now);
	obj->SetTimeOut(timeVal);
	t = obj->GetTimeOut();

	if (t >= m_AglieMidValue)
	{
		RInsertToList(t, obj);
	}
	else
	{
		InsertToList(t, obj);
	}
}

void CTimerManager::AttachTimerWithAgileTime(TimeVal t, ITimerObject* obj)
{
	t.TimeValAdd(m_Now);
	obj->SetTimeOut(t);
	float f = obj->GetTimeOut();
	
	if (f >= m_AglieMidValue)
	{
		RInsertToList(f, obj);
	}
	else
	{
		InsertToList(f, obj);
	}
}

CTimerList* CTimerManager::GetTimeList(int key)
{
	map<int, CTimerList*>::iterator iter = m_FixTimerMap.find(key);
	CTimerList* l = NULL;
	if (iter != m_FixTimerMap.end())
	{
		l = iter->second;
	}
	else
	{
		l = new CTimerList();
		l->SetType(TimerListFixed);
		m_FixTimerMap[key] = l;
	}

	return l;
}

void CTimerManager::InsertToList(float t, ITimerObject* obj)
{
	m_AglieTimerList->Insert(obj, t);
}

void CTimerManager::RInsertToList(float t, ITimerObject* obj)
{
	m_AglieTimerList->RInsert(obj, t);
}

}
