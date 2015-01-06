
// haj_gameevents.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gameevents.h"

#ifdef CLIENT_DLL
	#include "c_team.h"
#else
	#include "team.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// defines
#define EVENT_ZONE_CAPTURED			"zone_captured"
#define EVENT_ZONE_UNDER_ATTACK		"zone_pointunderattack"
#define EVENT_GLOBAL_SOUND			"global_sound"

/////////////////////////////////////////////////////////////////////////////
// globals
CHajGameEvents _gameEvents;

/////////////////////////////////////////////////////////////////////////////
#ifdef CLIENT_DLL

/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::SetupEventListeners()
{
	gameeventmanager->AddListener(this, EVENT_ZONE_CAPTURED, false);
	gameeventmanager->AddListener( this, EVENT_ZONE_UNDER_ATTACK, false );
	gameeventmanager->AddListener( this, EVENT_GLOBAL_SOUND, false );
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::FireGameEvent(IGameEvent* pEvent)
{
	assert(pEvent);

	// check event type and print message
	const char* szEventName = pEvent->GetName();
	int eventNameLen = strlen(szEventName);

	if(eventNameLen == 13)
	{
		if(!strcmp(EVENT_ZONE_CAPTURED, szEventName))
			OnEvent_ControlPointCaptured(pEvent);
	}

	if( !strcmp( EVENT_ZONE_UNDER_ATTACK, szEventName ) )
	{
		OnEvent_ControlPointUnderAttack( pEvent );
	}

	if( !strcmp( EVENT_GLOBAL_SOUND, szEventName ) )
	{
		CBasePlayer *pLocal = CBasePlayer::GetLocalPlayer();

		if( pLocal )
			pLocal->EmitSound( pEvent->GetString( "sound" ) );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::OnEvent_ControlPointCaptured(IGameEvent* pEvent)
{
	const int maxMsg = 256;
	char szMsg[maxMsg];
	char szNames[maxMsg] = "";
	const char *szZone;

	// if the local player is on the same team as the capturing
	// team, then allow him to see who captured the control point
	// otherwise, just display the name of the team
	int iTeam = pEvent->GetInt("teamid");
	szZone = pEvent->GetString("zone");

	C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	int localPlayerTeam = pLocalPlayer->GetTeamNumber();
	
	if(localPlayerTeam == iTeam)
	{
		// assemble a list of the capturing players
		int nPlayers = pEvent->GetInt("players");
		for(int iPlayer = 0 ; iPlayer < nPlayers ; ++iPlayer)
		{
			char useridstring[9];
			sprintf( useridstring, "userid%i", iPlayer );

			int userId = pEvent->GetInt( useridstring );
			int index = engine->GetPlayerForUserID(userId);
			C_BasePlayer* pPlayer = UTIL_PlayerByIndex(index);

			const char* szPlayerName = NULL;

			if(pPlayer)
			{
				szPlayerName = pPlayer->GetPlayerName();
			}
			else
			{
				// something went wrong
				assert(false);

				static char szNoName[] = "???";
				szPlayerName = szNoName;
			}

			assert(szPlayerName);
			Q_strcat(szNames, szPlayerName, maxMsg);

			// check if we need to seperate player names
			if(nPlayers > 1 && iPlayer < nPlayers - 1)
			{
				// check if we should have a comma
				if(nPlayers > 2 && nPlayers)
					Q_strcat(szNames, ", ", maxMsg);

				if(iPlayer == nPlayers - 2)
				{
					if(nPlayers == 2)
						Q_strcat(szNames, " and ", maxMsg);
					else
						Q_strcat(szNames, "and ", maxMsg);
				}
			}
		}
	}
	else
	{
		const char* szTeamName = g_Teams[iTeam]->Get_Name();
		Q_strncpy(szNames, szTeamName, maxMsg);
	}
	
	Q_snprintf(szMsg, maxMsg, "%s (team %d) captured a control point %s", szNames, iTeam, szZone);

	// send the message to the console
	Msg("%s\n", szMsg);

	/*
	// send the message to the events hud
	CHudEvents* pHudEvents = (CHudEvents*)gHUD.FindElement("CHudEvents");
	assert(pHudEvents);

	if(pHudEvents)
	{
		int texId = pHudEvents->GetTeamFlagTextureId(iTeam);
		pHudEvents->QueueEvent(texId, szMsg);
	}
	*/
}


/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::OnEvent_ControlPointUnderAttack(IGameEvent* pEvent)
{
	Msg( "%s is under attack from team %d\n", pEvent->GetString( "zone" ), pEvent->GetInt( "teamid") );
}

/////////////////////////////////////////////////////////////////////////////
#else	// CLIENT_DLL

/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::ControlPointCaptured(int capturingTeamId, int originalTeam, const CUtlVector<CBasePlayer*>& players, const char *szCapturePointName, int iEntIndex )
{
	int nPlayers = players.Count();

	IGameEvent* pEvent = gameeventmanager->CreateEvent(EVENT_ZONE_CAPTURED);

	if(!pEvent)
	{
		// event was not found. be sure the event is
		// defined in resources/modevents.res
		ASSERT(false);
		return;
	}

	// set the id of the capturing team
	pEvent->SetInt("teamid", capturingTeamId);
	pEvent->SetInt("origteam", originalTeam );
	pEvent->SetString("zone", szCapturePointName );
	pEvent->SetInt( "entityid", iEntIndex );

	// set the numer of userid. each userid represents a player
	// that helped capture the control point
	pEvent->SetInt("players", nPlayers);

	for(int i = 0 ; i < nPlayers ; ++i)
	{
		char szKey[32];
		Q_snprintf(szKey, sizeof(szKey), "userid%d", i);

		int playerId = players[i]->GetUserID();
		pEvent->SetInt(szKey, playerId);
	}
	
	// gogogo!
	gameeventmanager->FireEvent(pEvent);
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameEvents::ControlPointUnderAttack( int capturingTeamId, int ownerTeam, const char *szCapturePointName, int iEntIndex )
{
	IGameEvent* pEvent = gameeventmanager->CreateEvent( EVENT_ZONE_UNDER_ATTACK );

	if(!pEvent)
	{
		// event was not found. be sure the event is
		// defined in resources/modevents.res
		ASSERT(false);
		return;
	}

	// set the id of the capturing team
	pEvent->SetInt( "teamid", capturingTeamId );
	pEvent->SetInt( "ownerteam", ownerTeam );
	pEvent->SetString("zone", szCapturePointName );
	pEvent->SetInt( "entityid", iEntIndex );

	// gogogo!
	gameeventmanager->FireEvent(pEvent);
}

#endif	// CLIENT_DLL
