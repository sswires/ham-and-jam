
// haj_capturepoint.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_capturepoint_c.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// network data table
IMPLEMENT_CLIENTCLASS_DT(C_CapturePoint, DT_CapturePoint, CCapturePoint)
	RecvPropInt(RECVINFO(m_ownerTeam)),
	RecvPropInt(RECVINFO(m_capturingTeam)),
	RecvPropFloat(RECVINFO(m_capturePercentage)),
	RecvPropBool( RECVINFO( m_bAxisCanCapture ) ), 
	RecvPropBool( RECVINFO( m_bCWealthCanCapture ) ),
	RecvPropInt( RECVINFO( m_nMinCWealthPlayers) ),
	RecvPropInt( RECVINFO( m_nMinAxisPlayers ) ),
END_RECV_TABLE()

/////////////////////////////////////////////////////////////////////////////
C_CapturePoint::C_CapturePoint()
{
	m_sortIndex = 0;
	m_ownerTeam = TEAM_INVALID;
	m_capturePercentage = 0.0f;
	m_bShowOnHud = true;

	_objectiveman.AddCapturePoint(this);
}

/////////////////////////////////////////////////////////////////////////////
C_CapturePoint::~C_CapturePoint()
{
	_objectiveman.RemoveCapturePoint(this);
}

bool C_CapturePoint::ShouldDrawAlert()
{
	int iCWealthRole = GetTeamRole(TEAM_CWEALTH);
	int iAxisRole = GetTeamRole(TEAM_AXIS);

	if( iAxisRole == OBJECTIVE_ROLE_LOCKED || iCWealthRole == OBJECTIVE_ROLE_LOCKED )
		return false;

	return (bool)( GetCapturePercentile() > 0.0f && GetOccupantsByTeam( GetCapturingTeamId() ) > 0 );
}

void C_CapturePoint::PostDrawIcon( int x, int y, int iconWidth, int iconHeight, int iconPadding )
{
	int iCWealthRole = GetTeamRole(TEAM_CWEALTH);
	int iAxisRole = GetTeamRole(TEAM_AXIS);
	int iCapturingTeam = TEAM_SPECTATOR;

	if( iCWealthRole == OBJECTIVE_ROLE_CAPTURE )
		iCapturingTeam = TEAM_CWEALTH;

	if( iAxisRole == OBJECTIVE_ROLE_CAPTURE )
		iCapturingTeam = TEAM_AXIS;

	int iCapturers = GetOccupantsByTeam( iCapturingTeam );
	int iMinPlayers = GetCapCountByTeam( iCapturingTeam );

	if( iCapturers > 0 || iMinPlayers > 1 )
	{
		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		vgui::HFont capFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "ObjectiveSmallprint", true );

		if( capFont )
		{
			wchar_t szText[150] = L"";

			if( iMinPlayers > 1 )
				_snwprintf(szText, sizeof( szText ), L"%d/%d", iCapturers, iMinPlayers );
			else
				_snwprintf(szText, sizeof( szText ), L"%d", iCapturers );

			int tw, th;
			vgui::surface()->GetTextSize( capFont, szText, tw, th );

			vgui::surface()->DrawSetTextColor( 255, 255, 255, ( (iCapturers > 0) ? 255 : 150 ) );
			vgui::surface()->DrawSetTextPos( x + YRES(3), y + iconHeight - YRES(5) - (th/2) );
			vgui::surface()->DrawSetTextFont( capFont );
			vgui::surface()->DrawPrintText( szText, wcslen( szText ) );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////