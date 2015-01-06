
// haj_objectivemanager.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_objectivemanager.h"
#include "haj_capturepoint.h"
#include "haj_gamerules.h"
#include "haj_team.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// globals
CHajObjectiveManager _objectiveman;

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC( CHajObjective )
	DEFINE_THINKFUNC( BrushThink ),

	DEFINE_KEYFIELD(m_zoneId,		      FIELD_INTEGER, "ZoneId"),
	DEFINE_KEYFIELD(m_sortIndex,          FIELD_INTEGER, "SortIndex"),
	DEFINE_KEYFIELD(m_nameOfZone,	      FIELD_STRING,  "ZoneName"),
	DEFINE_KEYFIELD(m_zoneVisible,	      FIELD_INTEGER, "Visible"),
	DEFINE_KEYFIELD(m_bShowOnHud,		  FIELD_BOOLEAN, "ShowOnHUD"),

	DEFINE_KEYFIELD(m_szIconNeutral,	  FIELD_STRING,  "HUD_Icon_Neutral"),
	DEFINE_KEYFIELD(m_szIconAllies,		  FIELD_STRING,  "HUD_Icon_Allies"),
	DEFINE_KEYFIELD(m_szIconAxis,		  FIELD_STRING,  "HUD_Icon_Axis"),
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
// network data table
IMPLEMENT_SERVERCLASS_ST(CHajObjective, DT_HajObjective)
	SendPropInt(SENDINFO(m_zoneId), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_sortIndex), 8, SPROP_UNSIGNED),
	SendPropStringT( SENDINFO( m_nameOfZone ) ),
	SendPropBool(SENDINFO(m_bShowOnHud)),

	SendPropStringT( SENDINFO( m_szIconNeutral ) ),
	SendPropStringT( SENDINFO( m_szIconAllies ) ),
	SendPropStringT( SENDINFO( m_szIconAxis ) ),

	SendPropArray3(SENDINFO_ARRAY3(m_playersInArea), SendPropEHandle(SENDINFO(m_playersInArea))),
END_SEND_TABLE()

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::Spawn()
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	Init();

	// check if we should draw geometry
	if(m_zoneVisible == 0)
		SetRenderMode(kRenderNone);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::Init()
{
	// setup as a volume trigger
	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_NOT_SOLID|FSOLID_TRIGGER);

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	// set size of the array that holds the number
	// of players in the zone by team
	int nTeams = g_Teams.Count();
	nTeams -= CHajTeam::GetFirstCombatTeamIndex();
	m_playersInTeam.SetSize(nTeams);

	for(int i = 0 ; i < nTeams ; ++i)
		m_playersInTeam[i] = 0;
}

/////////////////////////////////////////////////////////////////////////////
const Vector& CHajObjective::GetAbsOrigin() const
{
	// note: another idea is to use the position of the flag associated
	// with the control point as the origin, and then fall back to the
	// center of the bounding box if there is no flag

	const CCollisionProperty* pCollision = CollisionProp();
	return pCollision->OBBCenter();
}

CHajObjective::CHajObjective()
{
	for(int i = 0 ; i < 32 ; ++i)
		m_playersInArea.Set(0, 0);

	m_playerList.SetLessFunc(CHajObjective::LessFunc);

	_objectiveman.AddObjective(this);
}

/////////////////////////////////////////////////////////////////////////////
CHajObjective::~CHajObjective()
{
	UnregisterWithTeam();

	_objectiveman.RemoveObjective(this);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::Activate()
{
	CBaseToggle::Activate();

	if(CHajTeam::IsCombatTeam( GetOwnerTeamId() ))
		RegisterWithTeam();
}

/////////////////////////////////////////////////////////////////////////////
CHajTeam* CHajObjective::GetOwnerTeam() const
{
	CHajTeam* pTeam = NULL;

	if( GetOwnerTeamId()  > TEAM_INVALID && GetOwnerTeamId() < TEAM_LAST && g_Teams.Count() > 0)
		pTeam = (CHajTeam*)g_Teams[GetOwnerTeamId()];

	return pTeam;
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::BrushThink()
{
	DoThink();

	// set next update
	m_lastThinkTime = gpGlobals->curtime;
	SetNextThink(gpGlobals->curtime + m_clientUpdateInterval);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::RegisterWithTeam()
{
	// register the control point with the team
	CHajTeam* pTeam = GetOwnerTeam();

	if(NULL != pTeam)
		pTeam->RegisterCapturePoint(this);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::UnregisterWithTeam()
{
	// unregister the control point with the team
	CHajTeam* pTeam = GetOwnerTeam();

	if(NULL != pTeam)
		pTeam->UnregisterCapturePoint(this);
}

/////////////////////////////////////////////////////////////////////////////
bool CHajObjective::IsPlayerInList(int userId)
{
	unsigned short idx = m_playerList.Find(userId);
	return m_playerList.IsValidIndex(idx);
}

/////////////////////////////////////////////////////////////////////////////
bool CHajObjective::AddPlayer(CBasePlayer* pPlayer)
{
	ASSERT(NULL != pPlayer);

	int userId = pPlayer->GetUserID();

	// add the player id if it is not already in the list
	if(!IsPlayerInList(userId))
	{
		int teamId = pPlayer->GetTeamNumber();
		teamId -= CHajTeam::GetFirstCombatTeamIndex();

		m_playerList.Insert(userId, userId);

		// add player entid to list for client
		m_playersInArea.Set(m_playerList.Count(), pPlayer);
		m_playersInTeam[teamId]++;

		// location update
		CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

		if( pHajPlayer )
		{
			pHajPlayer->UpdateLocation( m_nameOfZone.Get().ToCStr() );
			pHajPlayer->SetCurrentObjective( this );
		}

		OnPlayerEnter( pPlayer );
		return true;
	}

	return false;
}


/////////////////////////////////////////////////////////////////////////////
void CHajObjective::RemovePlayer(CBasePlayer* pPlayer)
{
	ASSERT(NULL != pPlayer);

	if( pPlayer )
	{
		int userId = pPlayer->GetUserID();

		if( !IsPlayerInList( userId ) )
			return;

		if( m_playerList.Remove(userId) )
		{
			int teamId = pPlayer->GetTeamNumber();
			teamId -= CHajTeam::GetFirstCombatTeamIndex();

			assert(teamId > -1 && teamId < m_playersInTeam.Count());
			m_playersInTeam[teamId]--;
			m_playersInTeam[teamId] = max(0, m_playersInTeam[teamId]);

			OnPlayerLeave( pPlayer );

			CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

			if( pHajPlayer )
				pHajPlayer->SetCurrentObjective( NULL );

		}
	}

	// reorganise the player list
	for(unsigned int i = 0 ; i < MAX_PLAYERS ; ++i)
	{
		if( m_playerList.IsValidIndex(i) )
		{
			CBasePlayer* pPlayer = UTIL_PlayerByUserId( m_playerList[i] );
			m_playersInArea.Set(i, pPlayer);
		}
		else
		{
			m_playersInArea.Set(i, NULL);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
void CHajObjective::StartTouch(CBaseEntity* pOther)
{
	if(!pOther->IsPlayer())
		return;

	CBasePlayer* pPlayer = ToBasePlayer(pOther);
	const char* szName = pPlayer->GetPlayerName();

	DevMsg("Enter: %s\n", szName);

	AddPlayer(pPlayer);

	// increase update rate
	m_clientUpdateInterval = TICK_INTERVAL;
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjective::EndTouch(CBaseEntity* pOther)
{
	if(!pOther->IsPlayer())
		return;

	CBasePlayer* pPlayer = ToBasePlayer(pOther);
	const char* szName = pPlayer->GetPlayerName();

	DevMsg("Exit: %s\n", szName);

	RemovePlayer(pPlayer);

	// reduce update rate if no players are within the area
	if(m_playerList.Count() > 0)
		m_clientUpdateInterval = CAPTUREPOINT_UPDATE_INTERVAL;
}

bool CHajObjective::PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info )
{
	if( pAttacker && gpGlobals->curtime >= pAttacker->m_flNextDefensiveAction && CanBeCapturedByTeam( GetTeamNumber() )
		&& GetOwnerTeamId() == pAttacker->GetTeamNumber() && pAttacker->GetTeamNumber() != GetTeamNumber() ) // the player isn't on your team
	{
		pAttacker->DefendedObjective( this, pVictim );
		pAttacker->SendNotification( "#HaJ_DefendPoint", NOTIFICATION_MENTIONS_OBJECTIVE, 3 + pAttacker->GetNearbyTeammates(), this, STRING( GetNameOfZone() ) );

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
CHajObjectiveManager::CHajObjectiveManager()
{

}

/////////////////////////////////////////////////////////////////////////////
CHajObjectiveManager::~CHajObjectiveManager()
{

}

/////////////////////////////////////////////////////////////////////////////
void CHajObjectiveManager::AddCapturePoint(CCapturePoint* pCapturePoint)
{
	m_capturePoints.AddToTail(pCapturePoint);
	AddObjective(pCapturePoint);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjectiveManager::RemoveCapturePoint(CCapturePoint* pCapturePoint)
{
	m_capturePoints.FindAndRemove(pCapturePoint);
	RemoveObjective(pCapturePoint);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjectiveManager::AddObjective(CHajObjective* pObjective)
{
	m_objectives.AddToTail(pObjective);
}

/////////////////////////////////////////////////////////////////////////////
void CHajObjectiveManager::RemoveObjective(CHajObjective* pObjective)
{
	m_objectives.FindAndRemove(pObjective);
}

/////////////////////////////////////////////////////////////////////////////
bool CHajObjectiveManager::HasTeamCompletedAllObjectives(int teamId) const
{
	// check if there are any objectives
	if(m_objectives.Count() < 1)
		return false;

	CUtlLinkedList<CHajObjective*>::IndexType_t it = m_objectives.Head();
	while(m_objectives.IsValidIndex(it))
	{
		CHajObjective* pObjective = m_objectives.Element(it);

		if(pObjective->GetOwnerTeamId() != teamId)
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CHajObjectiveManager::HasTeamCapturedAllControlPoints(int teamId) const
{
	// check if there are capture points
	if(m_capturePoints.Count() < 1)
		return false;

	CUtlLinkedList<CCapturePoint*>::IndexType_t it = m_capturePoints.Head();
	while(m_capturePoints.IsValidIndex(it))
	{
		CCapturePoint* pCapturePoint = m_capturePoints.Element(it);

		if(pCapturePoint->GetOwnerTeamId() != teamId)
			return false;
	}

	return true;
}
