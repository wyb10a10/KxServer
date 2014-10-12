#include <iostream>
#include <vector>
#include <algorithm>
#include "KXServer.h"

using namespace std;
using namespace KxServer;

static int TObjCount = 0;

class TimerObject : public ITimerObject
{
public:

	TimerObject()
	{
		++TObjCount;
		cout << "Create Timer Obj " << this << " TObjCount Count " << TObjCount << endl;
	}

	virtual ~TimerObject()
	{
		--TObjCount;
		cout << "Destroy Timer Obj" << this << " TObjCount Count " << TObjCount << endl;
	}

	virtual void OnTimer(const TimeVal& now)
	{
		cout << "OnTimer TimerObject Id " << id << endl;
	}

	int id;
};

static int RpObjCount = 0;

class RepeatTimerObject : public IRepeatTimeObject
{
public:

	RepeatTimerObject()
	{
		time = 0;
		++RpObjCount;
		cout << "Create Timer Obj" << this << " RpObjCount Count " << RpObjCount << endl;
	}

	virtual ~RepeatTimerObject()
	{
		--RpObjCount;
		cout << "Destroy Timer Obj" << this << " RpObjCount Count " << RpObjCount << endl;
	}

	virtual void OnTimer(const TimeVal& now)
	{
		IRepeatTimeObject::OnTimer(now);
		++time;
		cout<<"OnTimer RepeatTimerObject Id " << id <<" Times " << time << endl;
	}

	int time;
	int id;
};

int main()
{
    //测试定时器
    CTimerManager* timerMgr = CTimerManager::GetInstance();

	for (int i = 0; i < 10; ++i)
	{
		TimerObject* obj = new TimerObject();
		obj->id = i;

		//每个定时器的超时时间都不同
		timerMgr->AttachTimerWithAgileTime((float)i, obj);
		obj->release();
	}

	for (int i = 0; i < 10; ++i)
	{
		TimerObject* obj = new TimerObject();
		obj->id = i + 10;

		//每个定时器的超时时间都一样
		timerMgr->AttachTimerWithFixTime(10.0f, obj);
		obj->release();
	}

	//添加一个重复定时器
	RepeatTimerObject* rpObj = new RepeatTimerObject();
	rpObj->id = 888168;
	rpObj->Init(3.0f, 5);
	timerMgr->AttachTimerWithFixTime(3.0f, rpObj);
	rpObj->release();

	//定时器是交由Server进行管理的
	CBaseServer* server = new CBaseServer();
	server->ServerStart();
	delete server;

	return 0;
}
