/*
		© 2010 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Base for mounted MGs (that are also predicted)
*/

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "haj_player_c.h"
#else	
	#include "haj_player.h"
#endif

#ifdef CLIENT_DLL
	#define CMountedGunBase C_MountedGunBase
#endif

class CMountedGunBase : public CBaseAnimating
{
public:

	DECLARE_CLASS( CMountedGunBase, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CMountedGunBase();

	virtual void Precache();
	virtual void Activate();

	// accessors
	bool IsEnabled( void ) { return m_bEnabled; }
	void SetEnabled( bool state ) { m_bEnabled = state; }
	CHajPlayer* GetController( void ) { return m_hController.Get(); }

	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_ONOFF_USE; }

	// interactivity
	virtual void DoMGThink();
	virtual void FireWeapon( CHajPlayer *pController );
	void DoThink( void ) { DoMGThink(); }

#ifdef CLIENT_DLL
	bool IsLocalPlayerController() { return ( m_hController.Get() != NULL && m_hController.Get() == C_HajPlayer::GetLocalHajPlayer() ); }
	bool ShouldPredict() { return IsLocalPlayerController(); }
#endif
	

private:
	
	CNetworkVar( float, m_flNextShoot ); // next time we can fire
	CNetworkVar( bool, m_bEnabled );
	CNetworkHandle( CHajPlayer, m_hController );

#ifndef CLIENT_DLL
	COutputEvent m_OnFired;
#endif

protected:

	// fields
	CNetworkVar( float, m_flDelay ); // delay between shots

};