
// haj_gamemode_destroy_roundbase.h
// This is more like COD/CS where there is a single bomb and is sudden death

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEMODE_SEARCHANDDESTROY
#define __INC_GAMEMODE_SEARCHANDDESTROY

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamemode.h"
#include "haj_roundtimeroverride.h"

class CBasePlayer;


// round-based is the most configurable through convars and is designed for 5v5s
// this gamemode requires only ONE bomb target to be destroyed, rather all of them
extern ConVar haj_rbdestroy_roundtime;
extern ConVar haj_rbdestroy_roundlimit;
extern ConVar haj_rbdestroy_planttime;
extern ConVar haj_rbdestroy_defusetime;
extern ConVar haj_rbdestroy_bombtime;
extern ConVar haj_sab_scorelimit;

/////////////////////////////////////////////////////////////////////////////
class CHajGameModeDestroyRoundBased : public CHajGameMode, public CRoundTimerOverride
{
public:
	DECLARE_CLASS(CHajGameModeDestroyRoundBased, CHajGameMode);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajGameModeDestroyRoundBased();
	~CHajGameModeDestroyRoundBased() {}

public:
	// CBaseEntity overrides
	void Activate();
	void Spawn();
	virtual const char* GetGamemodeName() { return "Sabotage"; }
	virtual bool IsSuddenDeath() { return false; }
	virtual bool IsRoundBased() { return true; }

	// CRoundTimerOverride overrides
	float GetRoundTimeLeft() { return m_timer; }

	// macro to get defending team from attacking team
	int GetDefendingTeam( void );

	int GetRoundLimit() { return haj_rbdestroy_roundlimit.GetInt(); }
	virtual int GetScoreLimit( void ) { return haj_sab_scorelimit.GetInt(); }

	virtual float GetPlantTime( void ) { return haj_rbdestroy_planttime.GetFloat(); }
	virtual float GetDefuseTime( void ) { return haj_rbdestroy_defusetime.GetFloat(); }
	virtual float GetBombTimer( void ) { return haj_rbdestroy_bombtime.GetFloat(); }

	// events
	virtual void OnBombPlant( CBasePlayer *pPlanter, CBombZone* pZone );
	virtual void OnBombExplode( CBombZone* pZone );
	virtual void OnBombDefuse( CBasePlayer *pDefuser, CBombZone* pZone );
	virtual bool CanPlantBomb( CBasePlayer *pPlanter, CBombZone *pZone = NULL );

	virtual void ResetRound( void );

public:
	void OnThink();

private:
	// private dec
	int		 m_iTeamAttacker;
	float	 m_timelimit;
	float	 m_timer;

	bool	 m_bPlanted;
};

/////////////////////////////////////////////////////////////////////////////
#endif
