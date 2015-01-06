
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

extern ConVar haj_destroy_roundlimit;
extern ConVar haj_destroy_scorelimit;

/////////////////////////////////////////////////////////////////////////////
class CHajGameModeDestroy : public CHajGameMode, public CRoundTimerOverride
{
public:
	DECLARE_CLASS(CHajGameModeDestroy, CHajGameMode);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajGameModeDestroy();
	~CHajGameModeDestroy() {}

public:
	// CBaseEntity overrides
	void Activate();
	void Spawn();
	virtual void Precache();
	virtual const char* GetGamemodeName() { return "Destroy"; }
	virtual const char* GetWinString( int iTeam );

	// CRoundTimerOverride overrides
	float GetRoundTimeLeft() { return m_timer; }
	virtual bool ShouldFreezeTime( bool bBomb );
	virtual void ResetRound( void );

	void ExtendRound( float time )
	{
		m_timer += time;
	}

	void OnThink();

	virtual float GetPlantTime( void ) { return m_flPlantTime; }
	virtual float GetDefuseTime( void ) { return m_flDefuseTime; }
	virtual float GetBombTimer( void ) { return m_flFuzeTime; }

	virtual void PlayerSpawn( CHajPlayer *pPlayer );

	virtual int	GetRoundLimit( void ) { return haj_destroy_roundlimit.GetInt() ; }
	virtual int GetScoreLimit( void ) { return haj_destroy_scorelimit.GetInt(); }

private:
	// private data
	int		 m_iTeamAttacker;
	float	 m_timelimit;
	float	 m_timer;
	string_t m_soundCWealthVictory;
	string_t m_soundAxisVictory;
	int		 m_iSpawnWithBombs;

	float m_flPlantTime, m_flDefuseTime, m_flFuzeTime;
};

/////////////////////////////////////////////////////////////////////////////
#endif
