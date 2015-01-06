//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose:  HaJ Map Settings Entity
// Notes:
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "baseentity.h"

class CMapSettingsEntity : public CBaseEntity
{
public:
	DECLARE_CLASS( CMapSettingsEntity, CBaseEntity );
	DECLARE_DATADESC();
	
	CMapSettingsEntity();
	virtual void Spawn();

	void InputSetCWealthRespawnTime( inputdata_t &inputdata );
	void InputSetAxisRespawnTime( inputdata_t &inputdata );

private:
	int m_nCommonwealthNation;
	int m_nAxisNation;
	int m_nCommonwealthUnit;
	int m_nAxisUnit;
	int m_nCommonwealthInsignia;
	int m_nAxisInsignia;
	int m_nYear;
	int m_nSeason;
	int m_nTheatre;

	int m_iAlliesWaveTime;
	int m_iAxisWaveTime;
};

inline CMapSettingsEntity *ToMapSettingsEntity( CBaseEntity *pEntity )
{
	if ( !pEntity || pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CMapSettingsEntity *>( pEntity );
}
