#include "BaseServer.h"
#include "TimerManager.h"

namespace KxServer {

CBaseServer::CBaseServer(void)
{
	m_IsRunning = true;
	m_Poller = NULL;
}

CBaseServer::~CBaseServer(void)
{
}

void CBaseServer::ServerStart()
{
	if (!ServerInit())
	{
		return;
	}

	ServerRun();

	ServerUninit();
}

//server run
void CBaseServer::ServerRun()
{
	CTimerManager* timerMgr = CTimerManager::GetInstance();

	while(m_IsRunning)
	{
		if (NULL != m_Poller)
		{
			m_Poller->Poll();
		}

		timerMgr->UpdateTimer();
	}
}

bool CBaseServer::ServerInit()
{
	return true;
}

//server uninit
void CBaseServer::ServerUninit()
{

}

}
