//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Player status panel that shows info on player's class/health
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_playerresource.h"
#include "c_team.h"

#include "haj_player_c.h"
#include "haj_gamerules.h"
#include "haj_mapsettings_enums.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar haj_cl_playerinfo( "haj_cl_playerinfo", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Show player info (nation/class/rank) on HUD" );

//-----------------------------------------------------------------------------
// Purpose: Displays current team as graphic + text
//-----------------------------------------------------------------------------
class CHudPlayerPanel : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudPlayerPanel, vgui::Panel );

public:
	CHudPlayerPanel( const char *pElementName );
	
	void Init( void );
	void VidInit( void );
	void ShouldDraw( bool draw );
	// void MsgFunc_Damage( bf_read &msg );

	virtual void Reset( void );
	virtual bool ShouldDraw( void );
	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void OnThink();
	virtual void Paint( void );

private:

	void			SetRankGraphic();
	void			SetClassGraphic();
	void			SetNationGraphic();
	
	// variables
	bool			m_bShow;		// show panel or not?
	Color			white()			{ return Color(255,255,255,255); }

	int				m_iTeam;		//	Team ID
	int				m_iRole;		//	Class ID
	int				m_iRank;		//	Role ID
	int				m_iCWNation;
	int				m_iAXNation;

	int				m_iOldTeam;		//	Team ID
	int				m_iOldRole;		//	Class ID
	int				m_iOldRank;		//	Role ID
	int				m_iOldCWNation;
	int				m_iOldAXNation;


	CHudTexture		*m_pBackground;				// background texture

	CHudTexture		*m_pFigureCWealth0;			// character textures
	CHudTexture		*m_pFigureCWealth1;
	CHudTexture		*m_pFigureCWealth2;
	
	CHudTexture		*m_pFigureAxis0;		
	CHudTexture		*m_pFigureAxis1;
	CHudTexture		*m_pFigureAxis2;

	CHudTexture		*m_pCWealthRank0;		// rank textures
	CHudTexture		*m_pCWealthRank1;
	CHudTexture		*m_pCWealthRank2;
	CHudTexture		*m_pCWealthRank3;
	CHudTexture		*m_pCWealthRank4;

	CHudTexture		*m_pAxisRank0;			// rank textures
	CHudTexture		*m_pAxisRank1;
	CHudTexture		*m_pAxisRank2;
	CHudTexture		*m_pAxisRank3;
	CHudTexture		*m_pAxisRank4;

	CHudTexture		*m_pFlagGerman;				// flags
	CHudTexture		*m_pFlagBritish;
	CHudTexture		*m_pFlagPolish;
	CHudTexture		*m_pFlagCanadian;


	CHudTexture		*m_pFigure;		
	CHudTexture		*m_pRank;		
	CHudTexture		*m_pFlag;		

	// RES variables
	
	// background texture
	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_bg_generic" );
	
	// image dimensions and pos
	CPanelAnimationVarAliasType( float, m_flImageWidth, "ImageWidth",  "48", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageHeight, "ImageHeight", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageX, "ImageX",  "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageY, "ImageY", "0", "proportional_float" );

	// rank image dimensions and pos
	CPanelAnimationVarAliasType( float, m_flRankImageX, "RankImageX",  "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flRankImageY, "RankImageY", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flRankImageWidth, "RankImageWidth",  "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flRankImageHeight, "RankImageHeight",  "16", "proportional_float" );

	// flag image dimensions and pos
	CPanelAnimationVarAliasType( float, m_flFlagImageX, "FlagImageX",  "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flFlagImageY, "FlagImageY", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flFlagImageWidth, "FlagImageWidth",  "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flFlagImageHeight, "FlagImageHeight",  "16", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerPanel );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPlayerPanel::CHudPlayerPanel( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudHaJPlayer")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetScheme("SourceScheme");
	
	SetHiddenBits( HIDEHUD_MISCSTATUS );
	
	m_bShow = true;
	m_iTeam = TEAM_UNASSIGNED;
	m_iRole = CLASS_UNASSIGNED;
	m_iRank = 0;
	m_iCWNation = NATION_CWEALTH_BRITAIN;
	m_iAXNation = NATION_AXIS_GERMANY;

	m_iOldTeam = -1;
	m_iOldRole = -1;
	m_iOldRank = -1;
	m_iOldCWNation = -1;
	m_iOldAXNation = -1;

	m_pFigure = NULL;
	m_pRank = NULL;
	m_pFlag = NULL;
}

void CHudPlayerPanel::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	
	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
}

void CHudPlayerPanel::ShouldDraw( bool draw )
{	
	m_bShow = draw;
}

bool CHudPlayerPanel::ShouldDraw( void )
{
	if( HajGameRules() && HajGameRules()->IsGameOver() )
		return false;

	return ( haj_cl_playerinfo.GetBool() && CHudElement::ShouldDraw() );
}

void CHudPlayerPanel::Init( void )
{	
	Reset();
	VidInit();
}

void CHudPlayerPanel::Reset( void )
{
	m_iTeam = TEAM_UNASSIGNED;
	m_iRole = CLASS_UNASSIGNED;
	m_iRank = 0;
	m_iCWNation = NATION_CWEALTH_BRITAIN;
	m_iAXNation = NATION_AXIS_GERMANY;

	m_iOldTeam = -1;
	m_iRole = -1;
	m_iRank = -1;
	m_iOldCWNation = -1;
	m_iOldAXNation = -1;
}

void CHudPlayerPanel::VidInit( void )
{	
	m_pBackground	= gHUD.GetIcon( m_szBgTexture );

	m_pFlagGerman		= gHUD.GetIcon( "hud_german_flag_pp" );
	m_pFlagBritish		= gHUD.GetIcon( "hud_british_flag_pp" );	
	m_pFlagPolish		= gHUD.GetIcon( "hud_polish_flag_pp" );
	m_pFlagCanadian		= gHUD.GetIcon( "hud_canadian_flag_pp" );	
}

void CHudPlayerPanel::OnThink( void )
{	
	// disable the panel by default.
	ShouldDraw ( false );
	SetVisible ( false );

	if ( !CHudElement::ShouldDraw() )
		return;
	
	if ( engine->IsDrawingLoadingImage() )
		return;

	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();

	// don't draw the panel if any of the below are true.
	if ( pLocalPlayer == NULL )
		 return;

	// handle people we're spectating
	if( pLocalPlayer->IsObserver() && (pLocalPlayer->GetObserverMode() == OBS_MODE_CHASE || pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE) )
	{
		if( pLocalPlayer->GetObserverTarget() && pLocalPlayer->GetObserverTarget()->IsPlayer() )
			pLocalPlayer = ToHL2MPPlayer( pLocalPlayer->GetObserverTarget() );
	}

	if ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return;

	if ( pLocalPlayer->GetCurrentClass() == CLASS_UNASSIGNED )
		return;

	if ( pLocalPlayer->IsAlive() == false )
		 return;

	// make our panel drawable.
	SetVisible ( true );
	ShouldDraw ( true );

	// grab the team number
	m_iTeam = pLocalPlayer->GetTeamNumber();
	m_iRole = pLocalPlayer->GetCurrentClass();
	m_iRank = pLocalPlayer->GetCurrentRank();

	// grab the nation
	CHajGameRules *pGameRules = HajGameRules();

	if( !pGameRules )
	{
		m_iCWNation = 0;
		m_iAXNation = 0;
	}
	else
	{
		m_iCWNation = pGameRules->GetCommonwealthNation();
		m_iAXNation = pGameRules->GetAxisNation();
	}

	SetRankGraphic();
	SetClassGraphic();
	SetNationGraphic();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the rank graphic only if its changed
//-----------------------------------------------------------------------------
void CHudPlayerPanel::SetRankGraphic()
{
	char szRankIcon[128];

	if ( m_iTeam == TEAM_CWEALTH )
	{
		switch ( m_iCWNation )
		{
			default:
			case NATION_CWEALTH_BRITAIN:
			case NATION_CWEALTH_CANADA:			
				Q_snprintf( szRankIcon, sizeof(szRankIcon), "british_rank_%d", m_iRank );
				break;
							
			case NATION_CWEALTH_POLAND:
				Q_snprintf( szRankIcon, sizeof(szRankIcon), "polish_rank_%d", m_iRank );
				break;
		}
	}

	// Axis
	else
	{
		switch ( m_iAXNation )
		{
			default:
			case NATION_AXIS_GERMANY:
				Q_snprintf( szRankIcon, sizeof(szRankIcon), "german_rank_%d", m_iRank );
				break;
		}
	}

	m_pRank = gHUD.GetIcon( szRankIcon );
	m_iOldRank = m_iRank;
}

//-----------------------------------------------------------------------------
// Purpose: Updates the rank graphic only if its changed
//-----------------------------------------------------------------------------
void CHudPlayerPanel::SetClassGraphic()
{
	if ( m_iRole == m_iOldRole && m_iTeam == m_iOldTeam )
		return;

	switch ( m_iRole )
	{
		case CLASS_RIFLEMAN:
			m_iTeam == TEAM_AXIS ? m_pFigure = gHUD.GetIcon( "figure_axis_1") : m_pFigure = gHUD.GetIcon( "figure_cwealth_1" );
			break;

		case CLASS_ASSAULT:
			m_iTeam == TEAM_AXIS ? m_pFigure = gHUD.GetIcon( "figure_axis_2") : m_pFigure = gHUD.GetIcon( "figure_cwealth_2" );
			break;

		case CLASS_SUPPORT:
			m_iTeam == TEAM_AXIS ? m_pFigure = gHUD.GetIcon( "figure_axis_3") : m_pFigure = gHUD.GetIcon( "figure_cwealth_3" );
			break;
	}

	m_iOldRole = m_iRole;
}

//-----------------------------------------------------------------------------
// Purpose: Updates the rank graphic only if its changed
//-----------------------------------------------------------------------------
void CHudPlayerPanel::SetNationGraphic()
{
	if( m_iTeam == m_iOldTeam )
		return;

	if ( m_iTeam == TEAM_AXIS && ( m_iAXNation == m_iOldAXNation ) )
		return;

	if ( m_iTeam == TEAM_CWEALTH && ( m_iCWNation == m_iOldCWNation ) )
		return;

	if ( m_iTeam == TEAM_CWEALTH )
	{
		switch ( m_iCWNation )
		{
			default:
			case NATION_CWEALTH_BRITAIN:
				m_pFlag = m_pFlagBritish;
				break;
			
			case NATION_CWEALTH_CANADA:
				m_pFlag = m_pFlagCanadian;
				break;
				
			case NATION_CWEALTH_POLAND:
				m_pFlag = m_pFlagPolish;
				break;
		}

		m_iOldCWNation = m_iCWNation;
		m_iOldAXNation = -1;
	}

	// Axis
	else
	{
		switch ( m_iAXNation )
		{
			default:
			case NATION_AXIS_GERMANY:
				m_pFlag = m_pFlagGerman;
				break;
		}

		m_iOldAXNation = m_iAXNation;
		m_iOldCWNation = -1;
	}

	m_iOldTeam = m_iTeam;
}

//-----------------------------------------------------------------------------
// Purpose: organises the panel
//-----------------------------------------------------------------------------
void CHudPlayerPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Draws the panel
//-----------------------------------------------------------------------------
void CHudPlayerPanel::Paint()
{
	// check if we should draw
	if ( !m_bShow )
		return;

	// this is wierd, sometimes it loses the pointer to the backgroud
	// if so, regrab it.
	if ( m_pBackground == NULL )
		m_pBackground =	gHUD.GetIcon( m_szBgTexture );
	
	if ( m_pBackground != NULL )
		m_pBackground->DrawSelf( 0, 0, GetWide(), GetTall(), white() );

	if ( m_pFigure != NULL )
		m_pFigure->DrawSelf( m_flImageX, m_flImageY, m_flImageWidth, m_flImageHeight, white() );

	if ( m_pRank != NULL )
		m_pRank->DrawSelf( m_flRankImageX, m_flRankImageY, m_flRankImageWidth, m_flRankImageHeight, white() );

	if ( m_pFlag != NULL )
		m_pFlag->DrawSelf( m_flFlagImageX, m_flFlagImageY, m_flFlagImageWidth, m_flFlagImageHeight, white() );
}
