
// haj_deployee.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_DEPLOYEE
#define __INC_DEPLOYEE

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef BASEANIMATING_H
	#include "baseanimating.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CHajDeployee : public CBaseAnimating
{
public:
	DECLARE_CLASS(CHajDeployee, CBaseAnimating);
	DECLARE_DATADESC();

public:
	// 'stuctors
	CHajDeployee();
	virtual ~CHajDeployee() {}

public:
	virtual void Activate();
	virtual void Spawn();
	virtual void Precache();
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() | m_iCaps; }

private:
	// entity data
	string_t	m_requiredPickup;		// name of the required pickup item

	// private data
	EHANDLE		m_hRequiredPickup;
	int			m_iCaps;
	bool		m_bDeployed;
};

/////////////////////////////////////////////////////////////////////////////
#endif
