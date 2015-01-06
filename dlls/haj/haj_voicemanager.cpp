
// haj_voicemanager.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "voice_gamemgr.h"
#include "haj_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar haj_alltalk_during_interval;

/////////////////////////////////////////////////////////////////////////////
class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		CHajGameRules *pGR = HajGameRules();

		if( pGR )
		{
			if( haj_alltalk_during_interval.GetBool() && (pGR->IsGameOver() || pGR->IsFreeplay()) )
				return true; // all talk at the end of the game or in free play
			
			if( pGR->IsSuddenDeath() || pGR->IsRoundBased() && pListener->IsAlive() && !pTalker->IsAlive() )
				return false; // can't listen to dead player's chat when you're alive in sudden death or in round (one life) based gamemodes
		}

		return (pListener->GetTeamNumber() == pTalker->GetTeamNumber());
	}
};

/////////////////////////////////////////////////////////////////////////////
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

/////////////////////////////////////////////////////////////////////////////
