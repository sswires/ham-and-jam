
// haj_gamemode.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEMODE
#define __INC_GAMEMODE

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef BASEENTITY_H
	#include "baseentity.h"
#endif

#include "haj_gamerules.h"
//#include "haj_capturepoint.h"

class CHajGameRules;
class CBombZone;
class CHajPlayer;

/////////////////////////////////////////////////////////////////////////////
class CHajGameMode : public CBaseEntity
{
public:
	DECLARE_CLASS(CHajGameMode, CBaseEntity );
	DECLARE_DATADESC();

public:
	// 'structors
	CHajGameMode() {}
	virtual ~CHajGameMode() {}

	DECLARE_NETWORKCLASS(); 

	virtual void Spawn();
	virtual void Precache();

	virtual void SetGametype();

	virtual const char* GetGamemodeName() = 0;
	virtual const char* GetWinString( int iTeam ) { return "#HaJ_Win_Generic"; }
	virtual bool IsSuddenDeath() { return false; } // is gamemode with sudden death?
	virtual bool IsRoundBased( void ) { return false; }

	virtual bool ShouldFreezeTime( void );

	//virtual void OnZoneCapture( CCapturePoint* pPoint ) {};
	
	virtual void OnBombPlant( CBasePlayer *pPlanter, CBombZone* pZone ) {}
	virtual void OnBombExplode( CBombZone* pZone ) {}
	virtual void OnBombDefuse( CBasePlayer *pDefuser, CBombZone* pZone ) {}
	virtual bool CanPlantBomb( CBasePlayer *pPlanter, CBombZone *pZone = NULL ) { if( !pZone ) return false; return true; }

	virtual float GetPlantTime( void ) { return 5.0f; }
	virtual float GetDefuseTime( void ) { return 5.0f; }
	virtual float GetBombTimer( void ) { return 35.0f; }

	virtual void PlayerSpawn( CHajPlayer *pPlayer );

	virtual int GetBasicGameType();

	virtual void PlayVictorySound(int winningTeamId);
	virtual void PlayCustomVictoryMusic( int winningTeamId );

	virtual void ResetRound( void ) {};
	virtual int GetScoreLimit( void ) { return 0; }

	// always transmit over network
	int UpdateTransmitState() {	return SetTransmitState(FL_EDICT_ALWAYS); }

	COutputEvent m_eOnCWealthWin;
	COutputEvent m_eOnAxisWin;
	COutputEvent m_eOnRoundEnd;
	COutputEvent m_eOnRoundReset;
	COutputEvent m_eOnGameEndScreen;
	COutputEvent m_eOnCWealthGameWin;
	COutputEvent m_eOnAxisGameWin;
	COutputEvent m_eOnGameDraw;

protected:
	bool m_bMusicOverride;

	// entity data
	string_t m_soundCWealthVictory;
	string_t m_soundAxisVictory;
	string_t m_soundStalemate;

};

/////////////////////////////////////////////////////////////////////////////
#endif
