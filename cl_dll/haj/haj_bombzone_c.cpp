
// haj_BombZone.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_bombzone_c.h"

#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// network data table
IMPLEMENT_CLIENTCLASS_DT(C_BombZone, DT_BombZone, CBombZone)
RecvPropInt(RECVINFO(m_ownerTeam)),
RecvPropInt(RECVINFO(m_capturingTeam)),
RecvPropBool( RECVINFO( m_bAxisCanCapture ) ),
RecvPropBool( RECVINFO( m_bCWealthCanCapture ) ),
RecvPropInt( RECVINFO( m_iBombStage ) ),
RecvPropInt( RECVINFO( m_iStages ) ),
RecvPropBool( RECVINFO( m_bDestroyed )),
RecvPropBool( RECVINFO( m_bPlanted )),
RecvPropEHandle( RECVINFO( m_hPlantedBomb )),
RecvPropFloat( RECVINFO( m_flExplodeTime )),
END_RECV_TABLE()

/////////////////////////////////////////////////////////////////////////////
C_BombZone::C_BombZone()
{
	m_sortIndex = 0;
	m_ownerTeam = TEAM_INVALID;
	m_capturePercentage = 0.0f;
	m_bShowOnHud = true;

	_objectiveman.AddObjective(this);
}

/////////////////////////////////////////////////////////////////////////////
C_BombZone::~C_BombZone()
{
	_objectiveman.RemoveObjective(this);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
bool C_BombZone::CanBeCapturedByTeam(int teamId) const
{
	if( m_bPlanted || m_bDestroyed )
		return false;

	if(teamId == TEAM_CWEALTH)
		return m_bCWealthCanCapture;

	else if(teamId == TEAM_AXIS)
		return m_bAxisCanCapture;

	return false;
}

objectiveroles_e C_BombZone::GetTeamRole( int iTeam )
{
	if( m_bDestroyed )
		return OBJECTIVE_ROLE_DESTROYED;
	else if( GetOwnerTeamId() == iTeam && m_bPlanted )
		return OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES;
	else if( GetOwnerTeamId() == iTeam && GetEnemyTeamRole( iTeam ) != OBJECTIVE_ROLE_LOCKED )
		return OBJECTIVE_ROLE_DEFEND;
	else if( m_bPlanted && GetOwnerTeamId() != iTeam )
		return OBJECTIVE_ROLE_PREVENT_DEFUSE;
	else if( !CanBeCapturedByTeam( iTeam ) )
		return OBJECTIVE_ROLE_LOCKED;

	return OBJECTIVE_ROLE_PLANT_EXPLOSIVES;
}

void C_BombZone::PostDrawIcon( int x, int y, int iconWidth, int iconHeight, int iconPadding )
{

	if( m_iStages > 1 && !m_bDestroyed )
	{
		float screenScale = ScreenHeight() / 720;

		int boxx = x + (3*screenScale);
		int boxy = y + iconHeight - (8*screenScale);

		int boxw = (15 * screenScale );
		int boxh = (5*screenScale);

		int innerPad = screenScale;

		// background
		vgui::surface()->DrawSetColor( 0, 0, 0, 150 );
		vgui::surface()->DrawFilledRect( boxx, boxy, boxx + boxw + (iconPadding/2), boxy + boxh );

		int boxWidth = ( boxw / m_iStages );

		// stages
		for( int i = 0; i < m_iStages; i++ )
		{
			int bombStage = m_iStages - i;

			if( m_iBombStage >= bombStage )
				vgui::surface()->DrawSetColor( 150, 150, 150, 100 );	
			else
				vgui::surface()->DrawSetColor( 255, 255, 255, 200 );

			vgui::surface()->DrawFilledRect( boxx + innerPad + (i*boxWidth), boxy + innerPad, boxx + (boxWidth * (i+1) ), boxy + boxh - innerPad );
		}
	}

	if( m_bPlanted )
	{
		float timerem = m_flExplodeTime - gpGlobals->curtime + 1;

		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		vgui::HFont timefont = vgui::scheme()->GetIScheme(scheme)->GetFont( "ObjectiveSmallprint", true );

		if( timefont )
		{
			wchar_t szText[32];
			_snwprintf( szText, sizeof( szText ), L"%02d", (int)timerem );

			int tw, th;
			vgui::surface()->GetTextSize( timefont, szText, tw, th );

			int tx, ty;
			tx = x + RoundFloatToInt( ( iconWidth / 2.0f ) - ( tw / 2.0f ) );
			ty = y + RoundFloatToInt( ( iconHeight / 2.0f ) - ( th / 2.0f ) );

			vgui::surface()->DrawSetTextFont( timefont );

			// drop shadow
			vgui::surface()->DrawSetTextColor( 0, 0, 0, 255 );
			vgui::surface()->DrawSetTextPos( tx+2, ty+2 );
			vgui::surface()->DrawPrintText( szText, wcslen( szText ) );

			vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
			vgui::surface()->DrawSetTextPos( tx, ty );
			vgui::surface()->DrawPrintText( szText, wcslen( szText ) );
		}
	}
}

bool C_BombZone::ShouldDrawAlert()
{
	return m_bPlanted;
}