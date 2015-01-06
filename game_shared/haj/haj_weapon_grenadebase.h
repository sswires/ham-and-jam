#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "haj_grenade_base.h"
#endif

#include "effect_dispatch_data.h"
#include "haj_weapon_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponGrenade C_WeaponGrenade
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponGrenade: public CHAJWeaponBase
{
	DECLARE_CLASS( CWeaponGrenade, CHAJWeaponBase );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponGrenade();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	virtual void CheckGrenadeKeySwitch( void );

	float   GetSpreadScalar() { return 3.5f; }

	virtual void	FakeHolster();
	virtual void	FakeDeploy();

	void	ExplodeGrenadeInHand( CBasePlayer *pOwner );
	bool	Reload( void );

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	virtual bool ShouldDrawPickup() { return false; }
	virtual CHAJGrenade* CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

	CHandle<CBaseCombatWeapon> m_hRestoreLastWeapon;
#else
	virtual bool	ShouldForceTextureCrosshair( void ) { return true; }
#endif

	virtual bool			AllowsAutoSwitchFrom( void ) const { return true; };

	void			ThrowGrenade( CBasePlayer *pPlayer );
	bool			IsPrimed( void );
	void			DropGrenade( CBasePlayer *pPlayer );
	virtual bool	CanCookGrenade( void ) { return true; }

	virtual float	GetThrowVelocity( void );
	virtual float	GetLobVelocity( void );

	virtual float GetFuseTimer( void );
	virtual float GetThrowPowerMultiplier( void ) { return 1.0f; }

	float GetAttackTime() { return m_flAttackStartTime; }

	virtual float GetDamageRadius( void );
	virtual bool HasPrimaryAmmo( void );

	virtual bool	Lower( void );
	virtual bool	Ready( void );


	CNetworkVar( float, m_flAttackStartTime );	// when the player started the throw
	CNetworkVar( float,	m_flAttackHoldTimer );	// how long the player has held the throw
	CNetworkVar( float,	m_flAttackReleaseTime );	// when the player released the throw
	CNetworkVar( bool, m_bUsingGrenadeKey );
	
private:

	void	RollGrenade( CBasePlayer *pPlayer );
	void	LobGrenade( CBasePlayer *pPlayer );
	
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CNetworkVar( bool,	m_bRedraw );	//Draw the weapon again after throwing a grenade
	CNetworkVar( int,	m_AttackPaused );
	CNetworkVar( bool,	m_fDrawbackFinished );
	CNetworkVar( bool, m_bPullback );

	CWeaponGrenade( const CWeaponGrenade & );

#ifndef CLIENT_DLL
	bool m_bSuicideNade;

	DECLARE_ACTTABLE();
#endif
};
