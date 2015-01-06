
// haj_mapentityfilter.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_mapentityfilter.h"
#include "checksum_crc.h"
#include "haj_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
CMapEntityFilter::CMapEntityFilter()
: m_filter(16, 0, 0, CompareFunc, KeyFunc)
{
	AddFilter("ai_network");
	AddFilter("ai_hint");
	AddFilter("env_soundscape");
	AddFilter("env_soundscape_proxy");
	AddFilter("env_soundscape_triggerable");
	AddFilter("env_sprite");
	AddFilter("env_sun");
	AddFilter("env_wind");
	AddFilter("env_fog_controller");
	AddFilter("func_wall");
	AddFilter("func_illusionary");
	AddFilter("info_node");
	AddFilter("info_target");
	AddFilter("info_node_hint");
	AddFilter("func_precipitation");
	AddFilter("shadow_control");
	AddFilter("sky_camera");
	AddFilter("trigger_soundscape");
	AddFilter("worldspawn");
	AddFilter("soundent");
	AddFilter("haj_gamerules");
	AddFilter("scene_manager");
	AddFilter("predicted_viewmodel");
	AddFilter("team_manager");
	AddFilter("squad_manager");
	AddFilter("event_queue_saveload_proxy");
	AddFilter("player_manager");
	AddFilter("player");
	AddFilter("haj_mapsettings");
}

/////////////////////////////////////////////////////////////////////////////
CMapEntityFilter::~CMapEntityFilter()
{

}

/////////////////////////////////////////////////////////////////////////////
void CMapEntityFilter::AddFilter(const char* szFilter)
{
	// compute a checksum of the filter string
	// and add it to the hashtable
	long checksum = ComputeChecksum(szFilter);
	m_filter.Insert(checksum);
}

/////////////////////////////////////////////////////////////////////////////
void CMapEntityFilter::ClearFilters()
{
	m_filter.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
long CMapEntityFilter::ComputeChecksum(const char* szBuffer)
{
	CRC32_t checksum;
	CRC32_Init(&checksum);
	CRC32_ProcessBuffer(&checksum, szBuffer, strlen(szBuffer));
	CRC32_Final(&checksum);
	return checksum;
}

/////////////////////////////////////////////////////////////////////////////
CBaseEntity* CMapEntityFilter::CreateNextEntity(const char *pClassname)
{
	return CreateEntityByName(pClassname);
}

/////////////////////////////////////////////////////////////////////////////
bool CMapEntityFilter::ShouldCreateEntity(const char *pClassname)
{
	// check if the class name is within the filter table
	// if it is, then do not create the entity, otherwise
	// create away
	if( HajGameRules()->GetGamemode() != NULL && Q_strcmp( pClassname, HajGameRules()->GetGamemode()->GetClassname() ) == 0 )
		return false;

	long checksum = ComputeChecksum(pClassname);
	UtlHashHandle_t handle = m_filter.Find(checksum);
	UtlHashHandle_t invalid = m_filter.InvalidHandle();
	return (invalid == handle);
}

/////////////////////////////////////////////////////////////////////////////
bool CMapEntityFilter::CompareFunc(const long& a, const long& b)
{
	return (a == b);
}

/////////////////////////////////////////////////////////////////////////////
unsigned int CMapEntityFilter::KeyFunc(const long& item)
{
	return item;
}

/////////////////////////////////////////////////////////////////////////////
