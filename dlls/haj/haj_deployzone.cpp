#include "cbase.h"
#include "triggers.h"
#include "haj_player.h"
#include "haj_deployzone.h"

LINK_ENTITY_TO_CLASS( haj_deployzone, CDeployZone );

BEGIN_DATADESC( CDeployZone )
	DEFINE_KEYFIELD( m_iStance, FIELD_INTEGER, "stance" ),
	DEFINE_KEYFIELD( m_fAngleMax, FIELD_FLOAT, "angle_tolerance" ),
	DEFINE_KEYFIELD( m_iTeamFilter, FIELD_INTEGER, "team_filter" ),
END_DATADESC()

void CDeployZone::Spawn()
{
	Msg( "Spawned deploy zone...\n" );
	BaseClass::Spawn();

	QAngle pAngles = GetAbsAngles();
	m_fMinYaw = AngleNormalizePositive( pAngles[ YAW ] - m_fAngleMax );
	m_fMaxYaw = AngleNormalizePositive( pAngles[ YAW ] + m_fAngleMax );

	Msg( "Yaw is %f (max %f) - %f / %f\n", pAngles[ YAW ], m_fAngleMax, m_fMinYaw, m_fMaxYaw );

	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_NOT_SOLID|FSOLID_TRIGGER);

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	SetRenderMode(kRenderNone);
}

void CDeployZone::StartTouch(CBaseEntity *pOther)
{
	CHajPlayer *pPlayer = ToHajPlayer( pOther );

	if( pPlayer && pPlayer->IsPlayer() && pPlayer->IsAlive() && !pPlayer->IsObserver() )
	{
			if( m_iTeamFilter > 0 && pPlayer->GetTeamNumber() != m_iTeamFilter )
				return;

			DevMsg( "%s entered deploy zone", pPlayer->GetPlayerName() );
			pPlayer->SetDeployZone( true, m_iStance );
	}

	BaseClass::StartTouch( pOther );
}

void CDeployZone::EndTouch(CBaseEntity *pOther)
{
	CHajPlayer *pPlayer = ToHajPlayer( pOther );

	if( pPlayer && pPlayer->IsPlayer() && pPlayer->IsAlive() && !pPlayer->IsObserver() )
	{
		DevMsg( "%s left deploy zone", pPlayer->GetPlayerName() );
		pPlayer->SetDeployZone( false, 0 );
	}

	BaseClass::EndTouch( pOther );
}