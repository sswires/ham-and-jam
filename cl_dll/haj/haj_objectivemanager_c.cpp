
// haj_objectivemanager.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_objectivemanager_c.h"
#include "haj_capturepoint_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_HajObjective, DT_HajObjective, CHajObjective)
	RecvPropInt(RECVINFO(m_zoneId)),
	RecvPropInt(RECVINFO(m_sortIndex)),
	RecvPropString( RECVINFO( m_nameOfZone ) ),
	RecvPropBool(RECVINFO(m_bShowOnHud)),

	RecvPropString( RECVINFO( m_szIconNeutral ) ),
	RecvPropString( RECVINFO( m_szIconAllies ) ),
	RecvPropString( RECVINFO( m_szIconAxis ) ),

	RecvPropArray3(RECVINFO_ARRAY(m_playersInArea), RecvPropEHandle(RECVINFO(m_playersInArea[0]))),
END_RECV_TABLE()

/////////////////////////////////////////////////////////////////////////////
const Vector& C_HajObjective::GetAbsOrigin() const
{
	// note: another idea is to use the position of the flag associated
	// with the control point as the origin, and then fall back to the
	// center of the bounding box if there is no flag

	const CCollisionProperty* pCollision = CollisionProp();
	return pCollision->OBBCenter();
}


/////////////////////////////////////////////////////////////////////////////
void C_HajObjective::ClientThink()
{
	bool bWasInArea = m_bLocalPlayerInArea;
	m_bLocalPlayerInArea = false;

	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

	// check if the local player is inside the capture point
	for(int i = 0 ; i < 32 ; ++i)
	{
		CBaseEntity* pEntity = m_playersInArea[i];
		if(pEntity)
		{
			if( pLocalPlayer && pLocalPlayer->IsObserver() && pLocalPlayer->GetObserverTarget() && pEntity == pLocalPlayer->GetObserverTarget() )
				m_bLocalPlayerInArea = true;
			else
			{
				CBasePlayer* pBasePlayer = ToBasePlayer(pEntity);
				if(pBasePlayer && pBasePlayer->IsLocalPlayer())
					m_bLocalPlayerInArea = true;
			}
		}
	}

	// check if player entered or left the area
	if(bWasInArea != m_bLocalPlayerInArea)
	{
		if(bWasInArea)
		{
			// player left area
			// TODO: show and update progress indicator on HUD
			OnLocalPlayerLeave();
		}
		else
		{
			// player entered area
			// TODO: hide capture progress indicator on HUD
			OnLocalPlayerEnter();
		}
	}
}

const char* C_HajObjective::GetTeamIcon( int iTeamID )
{
	switch( iTeamID )
	{
	case TEAM_AXIS:
		if ( !Q_stricmp( GetAxisIcon(), "" ) )
			return "hud/obj_axis_default";
		else
			return GetAxisIcon();
		break;

	case TEAM_CWEALTH:
		if ( !Q_stricmp( GetAlliesIcon(), "" ) )
			return "hud/obj_allies_default";
		else
			return GetAlliesIcon();
		break;

	default:
		if ( !Q_stricmp( GetNeutralIcon(), "" ) )
			return "hud/obj_neutral_default";
		else
			return GetNeutralIcon();
		break;
	}

	return "hud/obj_neutral_default";
}

/////////////////////////////////////////////////////////////////////////////
void C_HajObjective::Spawn()
{
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

int C_HajObjective::GetOccupantsByTeam( int teamid )
{
	int count = 0;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		EHANDLE hPlayer = m_playersInArea[i];

		if( hPlayer )
		{
			C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer*>((C_BaseEntity*)hPlayer);

			if( pPlayer )
			{
				if( pPlayer->GetTeamNumber() == teamid )
				{
					count++;
				}
			}
		}
	}

	return count;
}

const char* C_HajObjective::GetStringForRole( void )
{
	if( C_BasePlayer::GetLocalPlayer() )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		int enemyRole = GetEnemyTeamRole( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() );

		switch( GetTeamRole( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() ) )
		{
				// We're meant to defend, so look at what the enemy has to do.
			case OBJECTIVE_ROLE_DEFEND:

				switch( enemyRole )
				{
					case OBJECTIVE_ROLE_LOCKED:
						return "#HaJ_CaptureEnemyCantCap";

					case OBJECTIVE_ROLE_CAPTURE:
						return "#HaJ_DefendArea";

					case OBJECTIVE_ROLE_PLANT_EXPLOSIVES:
						return "#HaJ_PreventPlant";

					default:
						return "#HaJ_DefendUnknownStatus";
				}
				break;

				// Locked :(
				case OBJECTIVE_ROLE_LOCKED:

					if( pPlayer->GetTeamNumber() == GetOwnerTeamId() && enemyRole == OBJECTIVE_ROLE_LOCKED )
						return "#HaJ_CaptureEnemyCantCap";
					else
						return "#HaJ_CaptureLocked";

				break;

				// We have to blow this up
			case OBJECTIVE_ROLE_PLANT_EXPLOSIVES:
				return "#HaJ_DestroyObjective";

			case OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES:
				return "#HaJ_DefuseExplosives";

			case OBJECTIVE_ROLE_DESTROYED:
				return "#HaJ_AlreadyDestroyed";

			case OBJECTIVE_ROLE_PREVENT_DEFUSE:
				return "#HaJ_ProtectBomb";

				// Hold this position (KOTH)
			case OBJECTIVE_ROLE_HOLD_POSITION:
				return "#HaJ_HoldHeadquarters";

				// Objective has been completed
			case OBJECTIVE_ROLE_COMPLETED:
				return "#HaJ_ObjectiveComplete";	

				// We're meant to capture this area
			case OBJECTIVE_ROLE_CAPTURE:

				// Display message if capture is blocked
				if( GetOccupantsByTeam( GetOwnerTeamId() ) > 0 )
				{
					return "#HaJ_CaptureBlocked";						
				}
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// globals
C_HajObjectiveManager _objectiveman;

/////////////////////////////////////////////////////////////////////////////
void C_HajObjectiveManager::AddCapturePoint(C_CapturePoint* pCapturePoint)
{
	m_capturePoints.AddToTail(pCapturePoint);
	AddObjective(pCapturePoint);
}

/////////////////////////////////////////////////////////////////////////////
void C_HajObjectiveManager::RemoveCapturePoint(C_CapturePoint* pCapturePoint)
{
	m_capturePoints.FindAndRemove(pCapturePoint);
	RemoveObjective(pCapturePoint);
}

/////////////////////////////////////////////////////////////////////////////
void C_HajObjectiveManager::AddObjective(C_HajObjective* pObjective)
{
	m_objectives.AddToTail(pObjective);
}

/////////////////////////////////////////////////////////////////////////////
void C_HajObjectiveManager::RemoveObjective(C_HajObjective* pObjective)
{
	m_objectives.FindAndRemove(pObjective);
}
