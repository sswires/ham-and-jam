
// haj_gameurles.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMERULES
#define __INC_GAMERULES

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "gamevars_shared.h"

#ifndef CLIENT_DLL
	#include "baseentity.h"
	#include "hl2mp_player.h"
	#include "haj_gamemode.h"
#else
	#include "c_baseentity.h"
	#include "haj_gamemode_c.h"
#endif

class CHajGameMode;

#include "utlmap.h"

// This is a basic enum for gamemode, there's only two options
enum {
	HAJ_GAMEMODE_PUSHPULL = 0, // there are no attack or defence, each team has both and attacking and defending role
	HAJ_GAMEMODE_ATTACKDEFEND // there's a clear attacking team, and defending team
};

// Custom kill reasons
enum {
	HAJ_KILL_GRENADE_IMPACT = 1,
	HAJ_KILL_SMOKEGREN_BURN,
	HAJ_KILL_CHOKING_HAZARD,
	HAJ_KILL_OBJECTIVE,
	HAJ_KILL_MG_OVERHEAT
};

/////////////////////////////////////////////////////////////////////////////
// defines
#define VEC_CROUCH_TRACE_MIN	HajGameRules()->GetCHajViewVectors()->m_vCrouchTraceMin
#define VEC_CROUCH_TRACE_MAX	HajGameRules()->GetCHajViewVectors()->m_vCrouchTraceMax

// *HAJ 020* - Jed
// Numeration of HaJ teamnames
enum
{
	// Unnassigned = 0, Spectator = 1 - defined elsewhere
	TEAM_CWEALTH = 2,
	TEAM_AXIS,
	TEAM_AUTO,
	TEAM_LAST,	// use this to check against upperlimit
};
// END HAJ

enum objectiveroles_e
{
	OBJECTIVE_ROLE_CAPTURE = 0,
	OBJECTIVE_ROLE_PLANT_EXPLOSIVES,
	OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES,
	OBJECTIVE_ROLE_PREVENT_DEFUSE,
	OBJECTIVE_ROLE_DEFEND,
	OBJECTIVE_ROLE_HOLD_POSITION,
	OBJECTIVE_ROLE_LOCKED,
	OBJECTIVE_ROLE_COMPLETED,
	OBJECTIVE_ROLE_DESTROYED,
	OBJECTIVE_ROLE_UNKNOWN, // should never be returned
};

enum itemType_e
{
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_LIMITED_ITEM,
};


struct inventoryItem_t
{
	char itemName[250];
	itemType_e itemType;
	int itemQuantity;
	int ammoIndex;

	inventoryItem_t()
	{
		itemQuantity = -1;
		ammoIndex = -1;
	}
};

#ifdef CLIENT_DLL
	#define CHajGameRules C_HajGameRules
	#define CHajGameRulesProxy C_HajGameRulesProxy
#endif

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CHajViewVectors;
class CRoundTimerOverride;
class CTeamInfo;

/////////////////////////////////////////////////////////////////////////////
class CHajGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CHajGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

/////////////////////////////////////////////////////////////////////////////
class CHajGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CHajGameRules, CTeamplayRules );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif

	CHajGameRules();
	virtual ~CHajGameRules();

	virtual void Precache( void );
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool ClientCommand( const char *pcmd, CBaseEntity *pEdict );

	virtual void Think( void );

#ifndef CLIENT_DLL
	void CheckTeamBalance( void );
	void CheckFreezeTimer( void );
	void RoundOverThink();

	bool HasReachedScoreLimit();
#endif

	virtual void CreateStandardEntities( void );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );

#ifndef CLIENT_DLL
	virtual const char *GetCustomKillString( const CTakeDamageInfo &info );
	virtual bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

	void	SendNotification( const char* szLocalise, int iNotificationType, int iFilterTeam = TEAM_INVALID, int iCE = 0, CBaseEntity *pEnt = NULL, const char* szEntName = NULL );
#endif

	virtual const char *GetGameDescription( void );
	// derive this function if you mod uses encrypted weapon info files
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"x9Ke0BY7"; }
	virtual const CViewVectors* GetViewVectors() const;
	const CHajViewVectors* GetCHajViewVectors() const;

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer ); // make it realistic only

	float	GetMapRemainingTime();
	float	GetRoundTimeLeft();

#ifndef CLIENT_DLL
	CUtlVector<char[75]> m_Accents[MAX_TEAMS];
	void	ParseTeamAccents( int iTeam );
	int		GetAccentCount( int iTeam);

	// map settings accessors, etc.
	void	SetCommonwealthNation( int n )	{ m_nCommonwealthNation = n; ParseTeamAccents( TEAM_CWEALTH ); };
	void	SetAxisNation( int n )			{ m_nAxisNation = n; ParseTeamAccents( TEAM_AXIS ); };
	void	SetCommonwealthUnit( int n )	{ m_nCommonwealthUnit = n; };
	void	SetAxisUnit( int n )			{ m_nAxisUnit = n; };
	void	SetCommonwealthInsignia( int n ){ m_nCommonwealthInsignia = n; };
	void	SetAxisInsignia( int n )		{ m_nAxisInsignia = n; };
	void	SetYear( int n )				{ m_nYear = n; };
	void	SetSeason( int n )				{ m_nSeason = n; };
	void	SetTheatre( int n )				{ m_nTheatre = n; };

#endif

	int		GetTeamNation( int i)			{ if( i == TEAM_CWEALTH ) return m_nCommonwealthNation; if( i == TEAM_AXIS ) return m_nAxisNation; return -1; }
	int		GetTeamUnit( int i )			{ if( i == TEAM_CWEALTH ) return m_nCommonwealthUnit; if( i == TEAM_AXIS ) return m_nAxisUnit; return -1; }
	int		GetCommonwealthNation()			{ return m_nCommonwealthNation; };
	int		GetAxisNation()					{ return m_nAxisNation; };
	int		GetCommonwealthUnit()			{ return m_nCommonwealthUnit; };
	int		GetAxisUnit()					{ return m_nAxisUnit; };
	int		GetCommonwealthInsignia()		{ return m_nCommonwealthInsignia; };
	int		GetAxisInsignia()				{ return m_nAxisInsignia; };
	int		GetYear()						{ return m_nYear; };
	int		GetSeason()						{ return m_nSeason; };
	int		GetTheatre()					{ return m_nTheatre; };

	void	AddInventoryItem( const char *entName, itemType_e itemType = ITEM_TYPE_WEAPON, int itemQuantity = -1, int iAmmoIndex = -1 );
	inventoryItem_t* GetInventoryItem( const char *entName );
	void	SetupInventoryItems( void );
	void	GetPlayerLoadout( CUtlVector<inventoryItem_t*> &items, int team, int pclass );

	const char* GetPlayerModel( int iTeam, int iClass );

	int		GetClampedStartingAmmoForClass( int iAmmoType, int iDefaultAmmo, int iTeam, int iClass );

	//Ztormi
	void	LoadMapScript( void );
	int		GetClasslimitForClass( int iClass, int iTeam );
	//---end

	virtual void ClientDisconnected( edict_t *pClient );

	bool IsIntermission( void );

	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	bool FPlayerCanRespawn( CBasePlayer *pPlayer );
	
	bool	IsTeamplay( void ) { return m_bTeamPlayEnabled;	}
	bool	IsGameOver( void ) { return m_bOnEndGameScreen; }

	virtual void SetGametype( int gametype ) { m_iGametype = gametype; };
	virtual int GetGametype( void ) { return m_iGametype; };

	void TeamVictoryResetRound(int winningTeamId);

	bool IsFreeplay() { return m_bFreeplay; }
	bool IsRoundBased();

	void StartOvertime( float time );
	void StartSuddenDeath( void );
	void SetSuddenDeath( bool bSud ) { m_bSuddenDeath = bSud; }

	bool IsInOvertime( void ) { return m_bOvertime; }
	bool IsSuddenDeath( void ) { return m_bSuddenDeath; }
	bool CanPlayerPlantBomb( CBasePlayer* pPlanter );

	float GetPlayerPlantTime( CBasePlayer* pPlanter );
	float GetPlayerDefuseTime( CBasePlayer* pDefuser );
	float GetBombTimer( CBasePlayer* pPlanter );

	bool IsFreezeTime( void ) { return m_bFreezeTime; }
	float GetRoundStartTime( void ) { return m_flRoundStartTime; }

	virtual bool  FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) { return false; }

#ifdef GAME_DLL

	virtual void FreezeAllPlayers( void );

	virtual bool CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem );
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );
	virtual CBaseCombatWeapon *GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon );
	virtual const char* GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	void ResetRound();

	void SetRoundTimerOverride(CRoundTimerOverride* pRoundTimer) { m_pRoundTimerOverride = pRoundTimer; }
	void SetGamemode( CHajGameMode *pGamemode ) 
	{
		if( pGamemode )
		{
			m_pGamemode = pGamemode;
			m_GamemodeHandle.Set( pGamemode );
		}
	}


	CBaseEntity* GetPlayerSpawnSpot(CBasePlayer* pPlayer);
#endif

	CTeamInfo *m_pTeamInfo[TEAM_LAST];
	bool m_bParsedTeamInfo;

	CTeamInfo* GetTeamInfo( int iTeam ) { return m_pTeamInfo[iTeam]; }
	void ParseTeamInfo( void );

	CHajGameMode* GetGamemode( void )
	{
#ifdef CLIENT_DLL
		return m_GamemodeHandle.Get();
#else
		return m_pGamemode;
#endif
	}
	
	CUtlVector<inventoryItem_t> m_PlayerItems;

private:
	CNetworkVar( bool, m_bTeamPlayEnabled );
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar(float, m_roundTimeLeft );

	CNetworkArray( bool, m_bWaitingForSpawn, TEAM_LAST );
	CNetworkArray( float, m_fSpawnTime, TEAM_LAST );

	CNetworkVar( int, m_iGametype );

	// Map settings vars.
	CNetworkVar( int, m_nCommonwealthNation );
	CNetworkVar( int, m_nAxisNation );
	CNetworkVar( int, m_nCommonwealthUnit );
	CNetworkVar( int, m_nAxisUnit );
	CNetworkVar( int, m_nCommonwealthInsignia );
	CNetworkVar( int, m_nAxisInsignia );
	CNetworkVar( int, m_nYear );
	CNetworkVar( int, m_nSeason );
	CNetworkVar( int, m_nTheatre );

	CNetworkVar( int, m_iRound );
	CNetworkVar( float, m_flRoundStartTime );
	CNetworkVar( bool, m_bOvertime );
	CNetworkVar( bool, m_bSuddenDeath );
	CNetworkVar( bool, m_bFreezeTime );

	CNetworkVar( bool, m_bFreeplay );
	CNetworkVar( bool, m_bOnEndGameScreen );

	//Classlimit vars - Ztormi
	CNetworkVar( int, m_iCommonwealthAssaultlimit );
	CNetworkVar( int, m_iCommonwealthRiflelimit );
	CNetworkVar( int, m_iCommonwealthMachinegunnerlimit );
	CNetworkVar( int, m_iWehrmachtAssaultlimit );
	CNetworkVar( int, m_iWehrmachtRiflelimit );
	CNetworkVar( int, m_iWehrmachtMachinegunnerlimit );
	//---end

	// Teams
	CNetworkVar( int, m_iAttackingTeam );
	CNetworkVar( int, m_iDefendingTeam );

	CNetworkHandle( CHajGameMode, m_GamemodeHandle );

	int m_iRespawnTimes[ TEAM_LAST ];

#ifdef GAME_DLL
	CRoundTimerOverride*	m_pRoundTimerOverride;
	CHajGameMode* m_pGamemode;

	bool m_bSwitchTeamsOnRoundReset;
	char m_szGamemodeName[75];
#endif

	float	m_resetRoundEndTime;
	bool	m_bRoundOver;
public:
	float	GetSpawnTimeForTeam( int iteam ) { return m_fSpawnTime.Get( iteam ); }
	int		GetRoundNumber( void ) { return m_iRound; }
	bool	IsRoundOver( void ) { return m_bRoundOver; }

	// accessors for teams, for push/pull mode, these should be TEAM_INVALID
	int		GetAttackingTeam( void ) { return m_iAttackingTeam; }
	void	SetAttackingTeam( int iTeam ) { m_iAttackingTeam = iTeam; }

	int		GetDefendingTeam( void ) { return m_iDefendingTeam; }
	void	SetDefendingTeam( int iTeam ) { m_iDefendingTeam = iTeam; }

#ifdef GAME_DLL
	void	BalanceTeams();
	void	SetRespawnTimes( int iAlliesTime, int iAxisTime )
	{
		m_iRespawnTimes[ TEAM_CWEALTH ] = iAlliesTime;
		m_iRespawnTimes[ TEAM_AXIS ] = iAxisTime;

		Msg( "Set spawn times - Axis: %d, Allies: %d\n", iAxisTime, iAlliesTime );
	}

	void	SetTeamRespawnTime( int iTeam, int iTime )
	{
		m_iRespawnTimes[ iTeam ] = iTime;
		Msg( "Set spawn time on team %d (2 = cwealth, 3 = axis) to %d\n", iTeam, iTime );
	}

	//CHajGameMode* GetGamemode( void ) { return dynamic_cast< CHajGameMode* >( m_pRoundTimerOverride ); }

	void ExtendTime( float fTime );
	virtual bool PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker );

private:
	float m_flTeamCheck;
	float m_flPerformTeamBalance;
#endif
};

/////////////////////////////////////////////////////////////////////////////
inline CHajGameRules* HajGameRules()
{
	return static_cast<CHajGameRules*>(g_pGameRules);
}

inline CHajGameRules* HL2MPRules()
{
	return HajGameRules();
}

/////////////////////////////////////////////////////////////////////////////
#endif
