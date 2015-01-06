//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Satchel detontation charge
// Note:	
// $NoKeywords: $
//=======================================================================//

#ifndef	WEAPONSATCHEL_H
#define	WEAPONSATCHEL_H

#include "basegrenade_shared.h"
#include "haj_weapon_base.h"

enum
{
	SATCHEL_ATTACHED_READY,
	SATCHEL_THROW,
	SATCHEL_ATTACH,
};

#ifdef CLIENT_DLL
#define CWeapon_Satchel C_Weapon_Satchel
#endif

class CWeapon_Satchel : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeapon_Satchel, CHAJWeaponBase );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CNetworkVar( int,	m_tSlamState );
	CNetworkVar( bool,	m_bDetonatorArmed );
	CNetworkVar( bool,	m_bNeedDetonatorDraw);
	CNetworkVar( bool,	m_bNeedDetonatorHolster);
	CNetworkVar( bool,	m_bNeedReload);
	CNetworkVar( bool,	m_bClearReload);
	CNetworkVar( bool,	m_bThrowSatchel);
	CNetworkVar( bool,	m_bAttachSatchel);
	CNetworkVar( bool,	m_bAttachTripmine);
	float				m_flWallSwitchTime;

	void				Spawn( void );
	void				Precache( void );

	void				PrimaryAttack( void );
	void				SecondaryAttack( void );
	void				WeaponIdle( void );
	void				Weapon_Switch( void );
	void				SatchelThink( void );
	
	void				SetPickupTouch( void );
	void				SatchelTouch( CBaseEntity *pOther );	// default weapon touch
	void				ItemPostFrame( void );	
	bool				Reload( void );
	void				SetSatchelState( int newState );

	bool				CanAttachSatchel( bool bAttack );
	bool				CanAttachSatchel(void) { return CanAttachSatchel( false ); }		// In position where can attach SLAM?
	bool				AnyUndetonatedCharges(void);
	void				StartTripmineAttach( void );
	void				TripmineAttach( void );

	void				StartSatchelDetonate( void );
	void				SatchelDetonate( void );
	void				StartSatchelThrow( void );
	void				StartSatchelAttach( void );
	void				SatchelThrow( void );
	void				SatchelAttach( void );
	bool				Deploy( void );
	bool				Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	CWeapon_Satchel();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
#endif

private:
	CWeapon_Satchel( const CWeapon_Satchel & );
};

#endif //WEAPONSATCHEL_H