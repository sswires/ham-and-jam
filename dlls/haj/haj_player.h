
// haj_player.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_PLAYER
#define __INC_PLAYER
#pragma once

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef HL2MP_PLAYER_H
	#include "hl2mp_player.h"
#endif

#include "haj_objectivemanager.h"
#include "haj_player_shared.h"
#include "haj_playerhelmet.h"
#include "haj_bombzone.h"
#include "haj_magazines.h"
#include "haj_playeranimstate.h"

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CHajPickup;
class CHajObjective;
class CBombZone;

enum notificationTypes_e
{
	NOTIFICATION_BASIC = 0,
	NOTIFICATION_MENTIONS_PLAYER,
	NOTIFICATION_MENTIONS_OBJECTIVE
};

/////////////////////////////////////////////////////////////////////////////
class CHajPlayer : public CHL2MP_Player
{
public:
	DECLARE_CLASS(CHajPlayer, CHL2MP_Player);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	// 'structors
	CHajPlayer();
	~CHajPlayer();

public:
	static CHajPlayer* CreatePlayer(const char* className, edict_t* ed)
	{
		CHajPlayer::s_PlayerEdict = ed;
		return (CHajPlayer*)CreateEntityByName(className);
	}

public:
	// returns the pickup item the player is currently carrying
	// returns NULL if the player isnt carrying anything
	CHajPickup* GetPickup() const;

	// called when a pickup object is touched by the player
	// returns false if the player rejects the pickup; otherwise returns true
	bool OnPickupTouch(CHajPickup* pPickup);
	bool OnPickupRecover( CHajPickup *pPickup );

	virtual void	Spawn( void );
	virtual void	InitialSpawn( void );
	virtual void	SuitPower_Update( void );

	virtual bool	IsValidObserverTarget(CBaseEntity * target); // true, if player is allowed to see this target
	virtual bool	StartObserverMode( int mode );

	virtual void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	void			PlayerDeathThink( );
	virtual void	PostThink( void );
	virtual void	SetAnimation( PLAYER_ANIM playerAnim );
	virtual void	ItemPostFrame();
	virtual void	UpdateLocation( const char* strLocation );
	virtual void	UpdateOnRemove( void );

// WEAPON STUFF
	virtual void	FireBullets( FireBulletsInfo_t &info, bool bLag = true );
	virtual void	HandleBulletPenetration( const FireBulletsInfo_t &info, unsigned short surfaceType, const trace_t &tr, const Vector &vecDir, ITraceFilter *pTraceFilter );
	virtual float	GetMaxPenetrationDistance( unsigned short surfaceType );
	virtual float	CalculatePenetrationDamageLoss( unsigned short surfaceType, float distanceTravelled );
	virtual void	CommitSuicide();

	// called when we elect to drop our weapon
	virtual bool	BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void	Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );

	virtual void	SetAmmoCrates( int crates ) { m_iAmmoCrates = crates; };
	virtual int		GetAmmoCrates( void ) { return m_iAmmoCrates; };
	virtual void	TakeAmmoCrates( int take ) { m_iAmmoCrates -= 1; };

// DEATH DAMAGE STUFF
	virtual int		BloodColor();
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual bool	HasHumanGibs( void ) { return true; }
	virtual bool	CorpseGib( const CTakeDamageInfo &info );

// HELMET STUFF
	void			PrecacheHelmets( void );
	void			CreateHelmet( void );		// create helmet entity
	void			DestroyHelmet( void );		// detstory helmet entity
	void			CreateHelmetGib( const CTakeDamageInfo &info, bool spawnHelmet, bool dropOnly );

	unsigned int	ChooseHelmet( void );
	unsigned int	ChooseCommonwealthHelmet( void );
	unsigned int	ChooseAxisHelmet( void );

	unsigned int	GetHelmet() { return m_nHelmetId; }
	void			SetHelmet( unsigned int id ) { m_nHelmetId = id; }
	
	virtual bool	CanSpeak( void );
	virtual bool	CanHearChatFrom( CBasePlayer *pPlayer );

	bool			CanVault();
	virtual bool	CanBreatheUnderwater() const { return false; }

	virtual void	Precache( void );

	virtual void	DoVoiceCommand( const char *strVoice );
	const char*		GetVoiceString( int iMenu, int iSelection );
	virtual bool	ClientCommand(const char *cmd);


	// *HAJ 020 - Jed
	// Use for lowering the weapon if not show for a while
	float GetLastShotTime( void ) { return m_flLastShotTime; }
	void  SetLastShotTime( float fTime );

	float			m_fNextTeamMateCheck;
	CNetworkVar( float, m_flLastShotTime );		// *HAJ 020* - Time last shot was fired
	
	void			UpdateNearbyTeamMateCount( void );
	
	virtual void	Event_Killed( const CTakeDamageInfo &info );

	virtual void	PlayerUse( void );

	virtual bool	Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon );
	virtual bool	IsWeaponLowered() { return m_bWeaponLowered; };
	virtual bool	Weapon_Lower( void );
	virtual bool	Weapon_Ready( void );

	virtual int					GiveAmmo( int nCount, int iAmmoIndex, bool bSuppressSound = false );
	CHajWeaponMagazines*		GetMagazines( int iAmmoType );

	void			SetRenderOrigin( Vector vecPos ) { m_vecRenderOrigin = vecPos; }

	void			SendNotification( const char* szLocalise, int iNotificationType, int iCE = 0, CBaseEntity *pEnt = NULL, const char* szEntName = NULL );

	unsigned int	GetNearbyTeammates() { return clamp( m_nNearbyTeamMates, 0, 3 ); }
	unsigned int	GetNearbyTeammatesUnclamped() { return m_nNearbyTeamMates; }

	void SetDeployZone( bool inzone, int stance )
	{
		DevMsg( "SetDeployZone called\n" );

		m_bInDeployZone = inzone;
		m_iDeployZStance = stance;
	}
	
	bool InDeployZone( void ) { return m_bInDeployZone; }
	int	 DeployZoneStance( void ) { return m_iDeployZStance; }

	int GetObjectiveScore( void ) { return m_iObjectiveScore; }
	void SetObjectiveScore( int iScore ) { m_iObjectiveScore = iScore; }
	void IncreaseObjectiveScore( int iIncrease ) { m_iObjectiveScore += iIncrease; }

	CNetworkVar( int, m_iObjectivesCapped );
	CNetworkVar( int, m_iObjectivesDefended );
	CNetworkVector( m_vecRenderOrigin );

	const char* GetPlayerLocation( void ) { return m_strLocation; }

	const char* GetPlayerLocation( bool bTeamChat );

	bool IsLocationSet( void ) { return ( m_strLocation[0] != '\0' ); }

	void ShowClassMenu( void );
	virtual bool HandleCommand_JoinTeam( int team );

	// Hints system
	void SendHint( const char* langkey );

	CNetworkVar( bool, m_bInBombZone );
	CNetworkHandle( CBombZone, m_hBombZone );

	bool IsInBombZone( void ) { return m_bInBombZone; }
	void SetInBombZone( bool b, CBombZone *pBombZone = NULL );

	bool CanUseVoiceCommands( void ) { return ( gpGlobals->curtime >= m_flNextVoiceCommand ); }

	virtual bool ShouldCollide( int collisionGroup, int contentsMask ) const;

	int m_iOpenVoiceMenu;
	int m_iAccent;
	int m_flNextDefensiveAction; // time of next defensive action
	int m_iAmmoGiven;

	CHajObjective* GetCurrentObjective( void ) { return m_pCurObjective; }
	void SetCurrentObjective( CHajObjective* pObj ) { m_pCurObjective = pObj; }

	void DefendedObjective( CHajObjective* pObj, CBasePlayer *pVictim, bool bDefuse = false );
	void KillOnPoint( CHajObjective* pObj, CBasePlayer *pVictim );

	CHajPlayerAnimState* GetAnimState( void ) { return m_pHajAnimState; }

private:
	// does this need to be networked?
	int				m_iAmmoCrates;
	CUtlVector<CHandle<CBasePlayer>> m_nearbyTeamMates;

	CHajObjective *m_pCurObjective;
	
	CNetworkVar( unsigned int, m_nNearbyTeamMates );
	CNetworkVar( bool, m_bWeaponLowered );
	CNetworkHandle( CBaseEntity, m_hPickup );

	// Helmet stuff
	CNetworkHandle( CBaseEntity, m_hHelmetEnt );	// tracks our server helmet entity
	CNetworkVar ( unsigned int, m_nHelmetId );		// ID for which helmet we're wearing
	bool m_bHasHelmet;								// Got a helmet on or not?

	float m_fNextLocationUpdate;
	char m_strLocation[70];
	char m_strLocationTeamChat[80];
	float m_flNextVoiceCommand;

	CHajPlayerAnimState *m_pHajAnimState;

	CUtlMap<int, CHajWeaponMagazines*> m_PlayerMagazines;

	CNetworkVar( bool, m_bInDeployZone );
	CNetworkVar( int, m_iDeployZStance );
	CNetworkVar( int, m_iObjectiveScore );
};

/////////////////////////////////////////////////////////////////////////////
inline CHajPlayer* ToHajPlayer(CBaseEntity* pEntity)
{
	if(!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<CHajPlayer*>(pEntity);
}

/////////////////////////////////////////////////////////////////////////////
#endif


// to determine assists we keep the damage a player gives/receives and seperate them per-life like DOD does.
struct PlayerAttack_t
{
	CHajPlayer*	pAttacker;
	CHajPlayer*	pVictim;

	int			iAttackerLife;
	int			iVictimLife;

	char		szAttackerName[150];
	char		szVictimName[150];
	char		szWeaponClass[150];

	int			iAttackerTeam;
	int			iVictimTeam;

	float		fDamage;
	float		fDistance;
	int			iDamageType;
	int			iHitGroup;
};

struct PlayerLifeRecord_t
{
	int	iCaptures;
	int iKills;
	int	iTeamKills;

	float fTotalDamage;

	CUtlVector< PlayerAttack_t > paPlayerDamageGiven;
	CUtlVector< PlayerAttack_t > paPlayerDamageTaken;
};