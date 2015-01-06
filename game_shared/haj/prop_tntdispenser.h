#include "cbase.h"

#ifdef CLIENT_DLL
#include "haj_player_c.h"

#define CTNTDispenser C_TNTDispenser
#else
#include "haj_player.h"
#endif

#define SF_START_DISABLED		(1 << 0)

class CTNTDispenser : public CBaseAnimating
{
public:

	DECLARE_CLASS( CTNTDispenser, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();

	CTNTDispenser();

	virtual void Precache();
	virtual void Activate();

#ifndef CLIENT_DLL
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void GiveTNT( CHajPlayer *pPlayer );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	virtual bool ShouldCollide( int collisionGroup, int contentsMask ) const;

	int ObjectCaps() 
	{ 
		return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
	}

	void Touch( CBaseEntity *pOther );
#else
	virtual int DrawModel( int flags );

	virtual ShadowType_t	ShadowCastType()
	{
		if( !m_bEnabled )
			return SHADOWS_NONE;

		return BaseClass::ShadowCastType();
	}
#endif


protected:

	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( int, m_iMaxGive );
	CNetworkVar( int, m_iLimitTeam );

#ifndef CLIENT_DLL
	float m_flPlayerTakeTime[MAX_PLAYERS];
#endif

};
