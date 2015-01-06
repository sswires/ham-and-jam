//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose: The team header section of the scoreboard.
// Note:	
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_player_c.h"
#include "haj_teaminfo.h"

#include <vgui/VGUI.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>

#include "haj_scoreboard_team.h"
#include "haj_gamerules.h"
#include "haj_mapsettings_enums.h"
#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CScoreboardTeamHeader::CScoreboardTeamHeader( vgui::VPANEL parent ) : BaseClass( NULL, "CScoreboardTeamHeader" )
{
	SetParent( parent );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
	InitControls();

	gameeventmanager->AddListener(this, "server_spawn", false );
}

CScoreboardTeamHeader::CScoreboardTeamHeader( vgui::Panel *parent, const char* panelName ) : vgui::EditablePanel( parent, panelName )
{
	InitControls( );
}

void CScoreboardTeamHeader::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_pTeamNameLabel->SetFont( m_TeamNameFont );
	m_pUnitNameLabel->SetFont( m_UnitNameFont );
	m_pScoreLabel->SetFont( m_ScoreFont );

	m_pFlagGerman		= gHUD.GetIcon( "hud_german_flag_sb" );
	m_pFlagBritish		= gHUD.GetIcon( "hud_british_flag_sb" );	
	m_pFlagPolish		= gHUD.GetIcon( "hud_polish_flag_sb" );
	m_pFlagCanadian		= gHUD.GetIcon( "hud_canadian_flag_sb" );	

	BaseClass::ApplySchemeSettings( pScheme );
}

CScoreboardTeamHeader::~CScoreboardTeamHeader()
{
}

void CScoreboardTeamHeader::InitControls( void )
{
	SetProportional( true );
	SetVisible( true );

	SetScheme( "ClientScheme" );

	// controls
	m_pTeamNameLabel = new vgui::Label( this, "teamname", "" );
	m_pUnitNameLabel = new vgui::Label( this, "unitname", "" );
	m_pPlayerCountLabel = new vgui::Label( this, "playercount", "" );
	m_pScoreLabel = new vgui::Label( this, "score", "" );
}

void CScoreboardTeamHeader::PerformLayout()
{
	m_pTeamNameLabel->SetPos( m_iTeamNameX, m_iTopPadding );
	m_pUnitNameLabel->SetPos( m_iTeamNameX, m_iUnitSpacing );

	m_pScoreLabel->SetFgColor( m_ScoreColor );
	m_pScoreLabel->SetPos( GetWide() - m_iScorePadding, m_iTopPadding );
	m_pScoreLabel->SetContentAlignment( vgui::Label::a_northeast );
}

void CScoreboardTeamHeader::Think( void )
{
	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( pLocalPlayer && !pLocalPlayer->IsDormant() )
	{
		if( g_PR )
		{
			m_pTeamNameLabel->SetFgColor( g_PR->GetTeamColor(GetTeam()) );
			m_pTeamNameLabel->SetText( g_PR->GetTeamName(GetTeam()) );
			m_pTeamNameLabel->SizeToContents();

			char scoreArray[10];
			Q_snprintf( scoreArray, sizeof( scoreArray), "%d", g_PR->GetTeamScore(GetTeam()) );

			m_pScoreLabel->SetText( scoreArray );
			m_pScoreLabel->SizeToContents();
		}

		// grab the nation
		CHajGameRules *pGameRules = HajGameRules();

		if( pGameRules )
		{
			if( IsVisible() )
			{
				pGameRules->ParseTeamInfo();
				CTeamInfo *pTeamInfo = pGameRules->GetTeamInfo( GetTeam() );

				if( pTeamInfo )
				{
					m_pUnitNameLabel->SetText( pTeamInfo->GetUnitName() );
					m_pUnitNameLabel->SizeToContents();
				}
			}

			int iCwealthNation = pGameRules->GetCommonwealthNation();
			int iAxisNation = pGameRules->GetAxisNation();

			if( GetTeam() == TEAM_CWEALTH )
			{
				switch( iCwealthNation )
				{
				case NATION_CWEALTH_CANADA:
					m_pFlag = m_pFlagCanadian;
					break;

				case NATION_CWEALTH_POLAND:
					m_pFlag = m_pFlagPolish;
					break;

				default:
					m_pFlag = m_pFlagBritish;
				}
			}
			else if( GetTeam() == TEAM_AXIS )
			{
				m_pFlag = m_pFlagGerman;
			}
		}
		else
		{
			m_pFlag = NULL;
		}
	}
}

void CScoreboardTeamHeader::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "server_spawn") == 0 )
	{
		CHajGameRules *pRules = HajGameRules();

		if( pRules )
			pRules->m_bParsedTeamInfo = false;
	}
}

void CScoreboardTeamHeader::Paint( void )
{
	PerformLayout();

	BaseClass::Paint();
}

void CScoreboardTeamHeader::PaintBorder()
{
	vgui::surface()->DrawSetColor( m_BorderColor );
	vgui::surface()->DrawLine( 0, GetTall() - 1, GetWide(), GetTall() - 1 );
}

void CScoreboardTeamHeader::PaintBackground( void )
{
	if( m_pFlag )
	{
		float flHeight = GetTall() - ( m_iIconPadding * 2 );
		m_pFlag->DrawSelf( m_iIconPadding, m_iIconPadding, flHeight * m_flIconAspect, flHeight, Color( 255, 255, 255, 255 ) );
	}
}