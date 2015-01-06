//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose:  Point entity that contains the name of the current location
// Notes:
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "triggers.h"
#include "baseentity.h"

class CHajLocation : public CBaseTrigger
{
public:
	DECLARE_CLASS( CHajLocation, CBaseTrigger );
	DECLARE_DATADESC();
	
	CHajLocation();
	virtual void Spawn();

	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	virtual const char* GetLocationName();

private:
	string_t m_iszLocationName;
};