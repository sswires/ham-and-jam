
// haj_pickup.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_PICKUP
#define __INC_PICKUP

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef BASEANIMATING_H
	#include "baseanimating.h"
#endif
#include "haj_player.h"

/////////////////////////////////////////////////////////////////////////////
class CHajPickup : public CBaseAnimating
{
public:
	DECLARE_CLASS(CHajPickup, CBaseAnimating);
	DECLARE_DATADESC();

public:
	// 'stuctors
	CHajPickup() {}
	virtual ~CHajPickup() {}

public:

	// CBaseEntity overrides
	virtual void Spawn();
	virtual void Precache();
	virtual void StartTouch(CBaseEntity* pOther);
	virtual void OwnerKilled( CHajPlayer* pPlayer, const CTakeDamageInfo &info );
	virtual void OnDropped();

	void Reset();
	bool IsDropped() { return m_bDropped; }
	virtual int GetCaptureTeam() { return m_iPickupTeam; }


private:
	bool m_bDropped;
	bool m_bBoneMerge;
	bool m_bAllowRecovery;

	Vector m_vecSpawn;
	QAngle m_angSpawn;

	int m_iPickupTeam; // -1 all, other is team id
};

/////////////////////////////////////////////////////////////////////////////
#endif
