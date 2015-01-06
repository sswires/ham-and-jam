
// haj_gamemode_pushpull.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEMODE_PUSHPULL
#define __INC_GAMEMODE_PUSHPULL

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef __INC_GAMEMODE
	#include "haj_gamemode.h"
#endif

#include "haj_roundtimeroverride.h"

extern ConVar haj_pp_roundlimit;
extern ConVar haj_pp_scorelimit;

/////////////////////////////////////////////////////////////////////////////
class CHajGameModePushPull : public CHajGameMode, public CRoundTimerOverride
{
public:
	DECLARE_CLASS(CHajGameModePushPull, CHajGameMode);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajGameModePushPull() { m_bMajorityWins = false; m_fOvertimeMultiplier = 1.25f; }
	~CHajGameModePushPull() {}

public:
	// CBaseEntity overrides
	virtual void Spawn();
	virtual void Precache();
	virtual const char* GetGamemodeName() { return "Push/Pull"; }
	virtual const char* GetWinString( int iTeam ) { return "#HaJ_Win_SecuredObjectives"; }

	virtual void ResetRound( void );

	virtual int GetBasicGameType() { return HAJ_GAMEMODE_PUSHPULL; }

public:
	void OnThink();

	// CRoundTimerOverride overrides
	float GetRoundTimeLeft();
	virtual int	GetRoundLimit( void ) { return haj_pp_roundlimit.GetInt() ; }
	virtual int GetScoreLimit( void ) { return haj_pp_scorelimit.GetInt(); }

private:

	bool m_bMajorityWins;
	bool m_bSuddenDeath;
	float m_fElapsedTime;
	float m_fOvertimeMultiplier;
};

/////////////////////////////////////////////////////////////////////////////
#endif
