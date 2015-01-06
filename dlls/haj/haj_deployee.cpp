
// haj_deployee.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_deployee.h"
#include "haj_deployablepickup.h"
#include "haj_pickup.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_deployee, CHajDeployee);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajDeployee)
	DEFINE_KEYFIELD(m_requiredPickup, FIELD_STRING,	"Pickup"),
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
CHajDeployee::CHajDeployee()
: m_bDeployed(false)
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CHajDeployee::Activate()
{
	BaseClass::Activate();

	// find the pickup in the world
	m_hRequiredPickup = dynamic_cast<CHajDeployablePickup*>(gEntList.FindEntityByName(NULL,
													m_requiredPickup,
													NULL));
}

/////////////////////////////////////////////////////////////////////////////
void CHajDeployee::Precache()
{
	PrecacheModel(STRING(GetModelName()));
}

/////////////////////////////////////////////////////////////////////////////
void CHajDeployee::Spawn()
{
	Precache();

	// set model
	SetModel(STRING(GetModelName()));

	// set to a state of continuous use
	m_iCaps	= FCAP_CONTINUOUS_USE;

	// no update. this entity is user-impulse driven
	SetNextThink(TICK_NEVER_THINK);

	// setup physics
	SetSolid(SOLID_VPHYSICS);
	VPhysicsInitStatic();
}

/////////////////////////////////////////////////////////////////////////////
void CHajDeployee::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if(m_bDeployed)
		return;

	// if it's not a player, ignore
	if(!pActivator || !pActivator->IsPlayer())
		return;

	CHajPlayer* pPlayer = ToHajPlayer(pActivator);
	assert(pPlayer);

	if(!pPlayer || !m_hRequiredPickup.Get())
		return;

	CHajDeployablePickup* pPickup = dynamic_cast<CHajDeployablePickup*>(pPlayer->GetPickup());
	if(!pPickup || m_hRequiredPickup.Get() != pPickup)
		return;
	
	// update deployment state
	pPickup->UpdateDeployment(gpGlobals->frametime);

	if(pPickup->IsCompletelyDeployed())
	{
		DevMsg("Depolyed!\n");
		m_bDeployed = true;
	}
	else
	{
		DevMsg("Deploying (%f)...\n", pPickup->GetDeployTimeLeft());
	}
}
