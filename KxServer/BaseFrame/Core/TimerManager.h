/* 
*   TimerManager 定时器管理器
*   1.定时器对象的定义
*   2.定时器的计时，触发
*   3.FixTime可以高效处理大量时间相等的定时器
*     例如每隔XX秒执行一次
*   4.AgileTime可以灵活处理各种时间不等的定时器
*     例如XX秒后执行一次
*     
*   2013-04-16 By 宝爷
*
*/
#ifndef __TIMERMANAGER_H__
#define __TIMERMANAGER_H__

#include <list>
#include <map>
#include "ICore.h"
#include "Ref.h"

namespace KxServer {

	enum TimerListType
	{
		TimerListAgile,
		TimerListFixed,
	};

class CTimerList;

//if you wan to managed by timermanager,inherit this interface
class ITimerObject : public CRef
{
public:

    ITimerObject();

    virtual ~ITimerObject();

	//pass current timer
	virtual void OnTimer(const TimeVal& now) = 0;

	//check is timeout, if timeout timermanager will remove from timerlist
	inline bool IsTimeOut(const TimeVal& now)
	{
		if ((now.tv_sec - m_TimeVal.tv_sec) + (now.tv_usec - m_TimeVal.tv_usec)/MILLION > 0.0f)
		{
			m_bIsAlive = false;
			return true;
		}

		return false;
	}

	inline bool IsTimeOut(float fnow)
	{
		if (fnow >= m_fTimeVal)
		{
			TimeVal timeVal;
			timeVal.SetFromfloat(fnow);
            m_bIsAlive = false;
			return true;
		}

		return false;
	}

	inline float GetTimeOut()
	{
		return m_fTimeVal;
	}

	//when you add timer to timermanager, this function will be call
	inline void SetTimeOut(TimeVal& t)
	{
		m_TimeVal = t;
		m_fTimeVal = m_TimeVal.Getfloat();
        m_bIsAlive = true;
	}

	inline void SetTimeOut(float t)
	{
		m_fTimeVal = t;
		m_TimeVal.SetFromfloat(t);
        m_bIsAlive = true;
	}

	inline bool IsAlive()
	{
		return m_bIsAlive;
	}

    //remove self from the timermanager, but on delete
    void RemoveSelf();
    
    friend class CTimerList;

private:
    CTimerList *m_TimerList;
    ITimerObject* m_Prev;
    ITimerObject* m_Next;

protected:
	TimeVal m_TimeVal;		//OnTimer will be called after this time
	float m_fTimeVal;		//For quick check
    bool m_bIsAlive;        //Timer Is Active
    bool m_bIsAutoRelease;  //Timer Will be delete when onTimer
};

class IRepeatTimeObject : public ITimerObject
{
public:
	IRepeatTimeObject();

	virtual void OnTimer(const TimeVal& now);

	virtual void Init(float delta, int repeat)
	{
		m_Delta = delta;
		m_RepeatTimes = repeat;
	}

private:
	int m_RepeatTimes;
	float m_Delta;
};

class CTimerList
{
public:
    
    CTimerList();

    virtual ~CTimerList();

    inline ITimerObject* Head()
    {
        return m_Head;
    }

    inline ITimerObject* Tail()
    {
        return m_Tail;
    }

    inline unsigned int Length()
    {
        return m_Length;
    }

    void PushBack(ITimerObject* obj);

    void PushFront(ITimerObject* obj);

    void Remove(ITimerObject* obj);

    void Insert(ITimerObject* obj, float timeOut);

    void RInsert(ITimerObject* obj, float timeOut);

    void Update(float fnow, const TimeVal& now);

	inline void SetType(TimerListType type)
	{
		m_Type = type;
	}

private:
    unsigned int m_Length;
	TimerListType m_Type;
    ITimerObject* m_Head;
    ITimerObject* m_Tail;
    ITimerObject* m_Timer;
};

class CTimerManager
{
private:
	CTimerManager(void);
	virtual ~CTimerManager(void);

public:
	static CTimerManager * GetInstance()
	{
		if (NULL == m_Instance)
		{
			m_Instance = new CTimerManager();
		}

		return m_Instance;
	}

	static void Destory()
	{
		delete m_Instance;
		m_Instance = NULL;
	}

	//call this function in main loop
	void UpdateTimer();

	// if you have many timerobject with the same delay time
	// or you wan't to repeat this timerobject more times, this function will be faster
	void AttachTimerWithFixTime(ITimerObject* obj);
	void AttachTimerWithFixTime(float t, ITimerObject* obj);
	void AttachTimerWithFixTime(TimeVal t, ITimerObject* obj);

	//if you just wan't update once, this function will be better
    void AttachTimerWithAgileTime(ITimerObject* obj);
	void AttachTimerWithAgileTime(float t, ITimerObject* obj);
	void AttachTimerWithAgileTime(TimeVal t, ITimerObject* obj);

	//insert to agile list from head to tail, please insert a "short time" obj
	void InsertToList(float t, ITimerObject* obj);
	//insert to agile list from tail to head, please insert a "long time" obj
	void RInsertToList(float t, ITimerObject* obj);

    inline float GetFNow()
    {
        return m_fNow;
    }

    inline const TimeVal& GetNow()
    {
        return m_Now;
    }

private:

    inline CTimerList* GetTimeList(int key);

private:
	//weight, if new time > aglie, add from tail, else add from head
	float										m_AglieMidValue;		
	float										m_fNow;
	TimeVal										m_Now;
	static CTimerManager*						m_Instance;
	CTimerList*         	    				m_AglieTimerList;
	std::map<int, CTimerList*>	                m_FixTimerMap;	
};

}

#endif
