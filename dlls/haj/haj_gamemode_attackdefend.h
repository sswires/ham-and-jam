
// haj_gamemode_attackdefend.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEMODE_ATTACKDEFEND
#define __INC_GAMEMODE_ATTACKDEFEND

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef __INC_GAMEMODE
	#include "haj_gamemode.h"
#endif
#ifndef __INC_ROUNDTIMEROVERRIDE
	#include "haj_roundtimeroverride.h"
#endif

extern ConVar haj_attackdefend_scorelimit;

/////////////////////////////////////////////////////////////////////////////
class CHajGameModeAttackDefend : public CHajGameMode, public CRoundTimerOverride
{
public:
	DECLARE_CLASS(CHajGameModeAttackDefend, CHajGameMode);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajGameModeAttackDefend();
	~CHajGameModeAttackDefend() {}

public:
	// CBaseEntity overrides
	void Activate();
	void Spawn();
	virtual void Precache();
	virtual const char* GetGamemodeName() { return "Attack/Defend"; }
	virtual const char* GetWinString( int iTeam );

	// CRoundTimerOverride overrides
	float GetRoundTimeLeft() { return m_timer; }
	virtual void ResetRound( void );

	void ExtendRound( float time )
	{
		m_timer += time;
	}

	virtual int GetScoreLimit( void ) { return haj_attackdefend_scorelimit.GetInt(); }

	void OnThink();

private:
	// private data
	int		 m_iTeamAttacker;
	float	 m_timelimit;
	float	 m_timer;
	float	 m_fOvertimeMultiplier;
	string_t m_soundCWealthVictory;
	string_t m_soundAxisVictory;
};

/////////////////////////////////////////////////////////////////////////////
#endif
