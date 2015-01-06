
// haj_spawnpoint.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_SPAWNPOINT
#define __INC_SPAWNPOINT

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef BASEENTITY_H
	#include "baseentity.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// defines
#define ENTITY_SPAWNPOINT_AXIS				"info_player_axis"
#define ENTITY_SPAWNPOINT_CWEALTH			"info_player_commonwealth"

/////////////////////////////////////////////////////////////////////////////
class CHajSpawnPoint : public CPointEntity
{
public:
	DECLARE_CLASS(CHajSpawnPoint, CPointEntity);
	DECLARE_DATADESC();

public:
	// 'structors
	CHajSpawnPoint();

public:
	void Spawn();
	bool IsEnabled() const { return m_bEnabled; }

	void OnPlayerSpawn(CBasePlayer* pPlayer);

protected:
	// input handlers
	void InputEnable(inputdata_t& inputdata);
	void InputDisable(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);

protected:	
	bool			m_bEnabled;
	COutputEvent	m_eOnPlayerSpawn;		// output event: "OnPlayerSpawn"
};

/////////////////////////////////////////////////////////////////////////////
#endif
