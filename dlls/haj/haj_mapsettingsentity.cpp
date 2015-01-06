//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose:  HaJ Map Settings Entity
// Notes:
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "haj_mapsettings_enums.h"
#include "haj_mapsettingsentity.h"
#include "haj_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( haj_mapsettings, CMapSettingsEntity );

BEGIN_DATADESC( CMapSettingsEntity )
	DEFINE_KEYFIELD( m_nCommonwealthNation,	FIELD_INTEGER,	"CommonwealthNation"),
	DEFINE_KEYFIELD( m_nAxisNation,			FIELD_INTEGER,	"AxisNation"),
	DEFINE_KEYFIELD( m_nCommonwealthUnit,	FIELD_INTEGER,	"CommonwealthUnit"),
	DEFINE_KEYFIELD( m_nAxisUnit,			FIELD_INTEGER,	"AxisUnit"),
	DEFINE_KEYFIELD( m_nCommonwealthInsignia,FIELD_INTEGER,	"CommonwealthUnit"),
	DEFINE_KEYFIELD( m_nAxisInsignia,		FIELD_INTEGER,	"AxisUnit"),
	DEFINE_KEYFIELD( m_nYear,				FIELD_INTEGER,	"Year"),
	DEFINE_KEYFIELD( m_nSeason,				FIELD_INTEGER,	"Season"),
	DEFINE_KEYFIELD( m_nTheatre,			FIELD_INTEGER,	"Theatre"),

	DEFINE_KEYFIELD( m_iAlliesWaveTime,		FIELD_INTEGER,	"CWealth_RespawnTime" ),
	DEFINE_KEYFIELD( m_iAxisWaveTime,		FIELD_INTEGER,	"Axis_RespawnTime" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetCWealthRespawnTime", InputSetCWealthRespawnTime ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetAxisRespawnTime", InputSetAxisRespawnTime ),
END_DATADESC()

// Constructor
CMapSettingsEntity::CMapSettingsEntity()
{
	// defaults
	m_nCommonwealthNation = 0;
	m_nAxisNation = 0;
	m_nCommonwealthUnit = 0;
	m_nAxisUnit = 0;
	m_nCommonwealthInsignia = 0;
	m_nAxisInsignia = 0;
	m_nYear = 0;
	m_nSeason = 0;
	m_nTheatre = 0;

	// team respawns
	m_iAlliesWaveTime = 0;
	m_iAxisWaveTime = 0;
}

// Entity spawn
void CMapSettingsEntity::Spawn()
{
	// get a pointer to the HAJ gamerules
	CHajGameRules* pGameRules = HajGameRules();
	if( !pGameRules) return;
	
	// push the values over
	pGameRules->SetCommonwealthNation( m_nCommonwealthNation );
	pGameRules->SetAxisNation( m_nAxisNation );
	pGameRules->SetCommonwealthUnit( m_nCommonwealthUnit );
	pGameRules->SetAxisUnit( m_nAxisUnit );
	pGameRules->SetCommonwealthInsignia( m_nCommonwealthInsignia );
	pGameRules->SetAxisInsignia( m_nAxisInsignia );
	pGameRules->SetYear( m_nYear );
	pGameRules->SetSeason( m_nSeason );
	pGameRules->SetTheatre( m_nTheatre );
	pGameRules->SetRespawnTimes( m_iAlliesWaveTime, m_iAxisWaveTime );

	//pGameRules->ParseTeamInfo();
}

void CMapSettingsEntity::InputSetCWealthRespawnTime( inputdata_t &inputdata )
{
	// get a pointer to the HAJ gamerules
	CHajGameRules* pGameRules = HajGameRules();
	if( !pGameRules) return;

	pGameRules->SetTeamRespawnTime( TEAM_CWEALTH, inputdata.value.Int() );
}

void CMapSettingsEntity::InputSetAxisRespawnTime( inputdata_t &inputdata )
{
	// get a pointer to the HAJ gamerules
	CHajGameRules* pGameRules = HajGameRules();
	if( !pGameRules) return;

	pGameRules->SetTeamRespawnTime( TEAM_AXIS, inputdata.value.Int() );
}