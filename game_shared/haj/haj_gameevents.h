
// haj_gameevents.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEEVENTS
#define __INC_GAMEEVENTS

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLVECTOR_H
	#include "utlvector.h"
#endif

#ifdef CLIENT_DLL
	#ifndef IGAMEEVENTS_H
		#include "igameevents.h"
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// forward declarations
#ifdef GAME_DLL
	class CBasePlayer;
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef GAME_DLL
	class CHajGameEvents
#else
	class CHajGameEvents : public IGameEventListener2
#endif
{
public:
#if GAME_DLL
	// event messages
	static void ControlPointCaptured(int capturingTeamId, int originalTeam, const CUtlVector<CBasePlayer*>& players, const char *szCapturePointName, int iEntIndex );
	static void ControlPointUnderAttack( int capturingTeamId, int ownerTeam, const char *szCapturePointName, int iEntIndex );
#else
	void SetupEventListeners();

	// client event handler
	void FireGameEvent(IGameEvent* pEvent);

	// client listerner functions
	void OnEvent_ControlPointCaptured(IGameEvent* pEvent);
	void OnEvent_ControlPointUnderAttack( IGameEvent* pEvent );
#endif
};

/////////////////////////////////////////////////////////////////////////////
// globals
extern CHajGameEvents _gameEvents;

/////////////////////////////////////////////////////////////////////////////
#endif
