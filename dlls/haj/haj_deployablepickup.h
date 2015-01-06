
// haj_deployablepickup.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_DEPOLYABLEPICKUP
#define __INC_DEPOLYABLEPICKUP

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef __INC_PICKUP
	#include "haj_pickup.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CHajDeployablePickup : public CHajPickup
{
public:
	DECLARE_CLASS(CHajDeployablePickup, CHajPickup);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajDeployablePickup();

public:
	float GetDeployTimeLeft() const { return m_timeleft; }
	bool IsCompletelyDeployed() const { return m_timeleft <= 0.0f; }
	void UpdateDeployment(float dt);

public:
	// CHajPickup overrides
	virtual void OnDropped() { ResetTimer(); }

	// CBaseEntity overrides
	virtual void Activate();

private:
	// resets the timer for the deploy state
	void ResetTimer();

private:
	// entity data
	float		m_deployTime;

	// private data
	float		m_timeleft;				// time remaining for deploy state
};

/////////////////////////////////////////////////////////////////////////////
#endif
