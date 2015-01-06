
// haj_gamemode_destroy.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamemode_destroy.h"
#include "haj_gamerules.h"
#include "haj_objectivemanager.h"
#include "haj_bombzone.h"
#include "haj_team.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_gamemode_destroy, CHajGameModeDestroy);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajGameModeDestroy)
DEFINE_THINKFUNC(OnThink),

DEFINE_KEYFIELD(m_iTeamAttacker,       FIELD_INTEGER,   "Attacker"),
DEFINE_KEYFIELD(m_timelimit,           FIELD_FLOAT,     "Timelimit"),
DEFINE_KEYFIELD(m_soundCWealthVictory, FIELD_STRING,	"CWealthVictorySound"),
DEFINE_KEYFIELD(m_soundAxisVictory,    FIELD_STRING,	"AxisVictorySound"),

DEFINE_KEYFIELD( m_flPlantTime,			FIELD_FLOAT,	"PlantTime"),
DEFINE_KEYFIELD( m_flDefuseTime,		FIELD_FLOAT,	"DefuseTime"),
DEFINE_KEYFIELD( m_flFuzeTime,			FIELD_FLOAT,	"BombTimer"),

DEFINE_KEYFIELD( m_iSpawnWithBombs,		FIELD_INTEGER,	"SpawnWithBombs"), // number of bombs to spawn with

END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
CHajGameModeDestroy::CHajGameModeDestroy()
{
	m_timelimit = m_timer = 30.0f;
	m_iTeamAttacker = TEAM_INVALID;

	m_flPlantTime = 5.0f;
	m_flDefuseTime = 5.0f;
	m_flFuzeTime = 35.0f;
	m_iSpawnWithBombs = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroy::Activate()
{
	BaseClass::Activate();

	m_timer = m_timelimit * 60.0f;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroy::OnThink()
{
	CHajGameRules* pGameRules = HajGameRules();

	if( !pGameRules )
		return;

	bool bBombPlanted = false;
	int nBombSites = 0;
	int nOwned = 0;

	// check if attacking team controls all the capture points
	if(CHajTeam::IsCombatTeam(m_iTeamAttacker))
	{
		CHajTeam* pTeam = (CHajTeam*)g_Teams[m_iTeamAttacker];

		const CUtlLinkedList<CHajObjective*>& objectives = _objectiveman.GetObjectives();
		unsigned short it = objectives.Head();

		while(objectives.IsValidIndex(it))
		{
			CBombZone* pObjective = (CBombZone*)objectives.Element(it);
			
			if( pObjective )
			{
				if( pObjective->GetOwnerTeamId() == m_iTeamAttacker )
					nOwned++;

				if( pObjective->IsBombPlanted() )
					bBombPlanted = true;

				nBombSites++;
			}

			it = objectives.Next(it);
		}

		if( nOwned == nBombSites ) // we got em
		{
			PlayVictorySound(m_iTeamAttacker);
			pGameRules->TeamVictoryResetRound(m_iTeamAttacker);
			return;
		}
	}

	// check if the timer has expired
	if( !ShouldFreezeTime(bBombPlanted) )
		m_timer -= gpGlobals->interval_per_tick;

	if( m_timer <= 0.0f && !bBombPlanted )
	{
		int iDefendingTeam = TEAM_CWEALTH;

		if(m_iTeamAttacker == TEAM_CWEALTH)
		{
			iDefendingTeam = TEAM_AXIS;
		}
		else if(m_iTeamAttacker == TEAM_AXIS)
		{
			iDefendingTeam = TEAM_CWEALTH;
		}
		else
		{
			// invalid team setup
			assert(false);
		}

		PlayVictorySound(iDefendingTeam);
		pGameRules->TeamVictoryResetRound(iDefendingTeam);

		return;
	}

	// update every frame
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroy::Spawn()
{
	CHajGameRules* pGameRules = HajGameRules();
	pGameRules->SetRoundTimerOverride(this);

	pGameRules->SetAttackingTeam( m_iTeamAttacker );

	if( m_iTeamAttacker == TEAM_CWEALTH )
		pGameRules->SetDefendingTeam( TEAM_AXIS );
	else
		pGameRules->SetDefendingTeam( TEAM_CWEALTH );

	SetThink(&CHajGameModeDestroy::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);

	BaseClass::Spawn();
}


void CHajGameModeDestroy::ResetRound( void )
{
	m_timer = m_timelimit * 60.0f;

	SetThink(&CHajGameModeDestroy::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}


const char* CHajGameModeDestroy::GetWinString( int iTeam )
{
	if( m_iTeamAttacker == iTeam )
		return "#HaJ_Win_ExplodedObjectives";

	return "#HaJ_Win_SuccessfulDestroyDefence";
}

void CHajGameModeDestroy::Precache()
{
}

void CHajGameModeDestroy::PlayerSpawn( CHajPlayer *pPlayer )
{
	if( pPlayer->GetTeamNumber() != m_iTeamAttacker )
		return;

	if( m_iSpawnWithBombs > 0 )
	{
		CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pPlayer->GiveNamedItem( "weapon_satchelcharge" );

		if( pWeapon )
			pPlayer->SetAmmoCount( m_iSpawnWithBombs, pWeapon->GetPrimaryAmmoType() );
	}
}

bool CHajGameModeDestroy::ShouldFreezeTime( bool bBomb )
{
	if( bBomb )
		return true;

	return BaseClass::ShouldFreezeTime();
}
/////////////////////////////////////////////////////////////////////////////
