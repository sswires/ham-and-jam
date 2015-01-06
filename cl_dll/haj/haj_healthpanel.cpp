//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Little panel that shows health and stamina
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"
#include "hl2mp_gamerules.h"
#include "c_team.h"
#include "haj_player_c.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1
#define HEALTH_CHUNKS 7
#define STAMINA_CHUNKS 7
#define TEAM_CHUNKS 3

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
class CHudHaJHealth : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudHaJHealth, vgui::Panel );

public:
	CHudHaJHealth( const char *pElementName );
	
	void Init( void );
	void VidInit( void );
	void ShouldDraw( bool draw );

	virtual void Reset( void );
	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void OnThink();
	virtual void Paint( void );

	void UpdateDisplay();
	void UpdatePlayerStatus( C_HL2MP_Player *player );

private:
	
	// variables
	bool			m_bShow;		// show panel or not?
	
	int				m_iHealth;		//	Player Health
	int				m_bitsDamage;	//  Damage bits (for effects)
	int				m_iStamina;		//	Player Stamina
	int				m_iTeammatesNearby; // Number of team mates nearby

	CHudTexture		*m_pBackground;		// background texture
	CHudTexture		*m_pIconHealth;		// health icon
	CHudTexture		*m_pIconHealthCrit;		// health icon
	CHudTexture		*m_pIconSprint;		// sprint icon
	CHudTexture		*m_pIconTeamwork;	// team mate indicator

	// layout vars
	float			m_flBarChunkWidth;
	float			m_flSBarChunkWidth;
	float			m_flTBarChunkWidth;

	// Panel variables.
	// TODO: Once the default layout is decided, copy the values from the RES file to here.
	
	// background texture
	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_bg_generic" );

	// health colours
	CPanelAnimationVar( Color, m_HealthLeftColor, "HealthLeftColor", "0 192 0 128" );
	CPanelAnimationVar( Color, m_HealthGoneColor, "HealthGoneColor", "120 0 0 128" );

	// stamina colours
	CPanelAnimationVar( Color, m_StaminaLeftColor, "StaminaLeftColor", "192 192 0 128" );
	CPanelAnimationVar( Color, m_StaminaGoneColor, "StaminaGoneColor", "64 64 0 128" );

	// team colours
	CPanelAnimationVar( Color, m_TeamLeftColor, "TeamLeftColor", "255 255 0 192" );
	CPanelAnimationVar( Color, m_TeamGoneColor,	"TeamGoneColor", "128 128 0 192" );
	
	// icon sizes
	CPanelAnimationVarAliasType( float, m_flIconWidth, "IconWidth",  "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconHeight, "IconHeight", "32", "proportional_float" );

	// icon postion
	CPanelAnimationVarAliasType( float, m_flHealthIconX, "HealthIconX",  "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flHealthIconY, "HealthIconY", "86", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSprintIconX, "SprintIconX",  "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSprintIconY, "SprintIconY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTeamIconX, "TeamIconX",  "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTeamIconY, "TeamIconY", "106", "proportional_float" );

	// health bar variables
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "14", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "10", "proportional_float" );
	CPanelAnimationVar( float, m_flBarChunkGap, "BarChunkGapRatio", "0.8" );

	// sprint bar variables
	CPanelAnimationVarAliasType( float, m_flSBarInsetX, "SprintBarInsetX", "14", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSBarInsetY, "SprintBarInsetY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSBarWidth, "SprintBarWidth", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSBarHeight, "SprintBarHeight", "10", "proportional_float" );
	CPanelAnimationVar( float, m_flSBarChunkGap, "SprintBarChunkGapRatio", "0.8" );

	// team bar variables
	CPanelAnimationVarAliasType( float, m_flTBarInsetX, "TeamBarInsetX", "14", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTBarInsetY, "TeamBarInsetY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTBarWidth, "TeamBarWidth", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTBarHeight, "TeamBarHeight", "10", "proportional_float" );
	CPanelAnimationVar( float, m_flTBarChunkGap, "TeamBarChunkGapRatio", "0.8" );
};

DECLARE_HUDELEMENT( CHudHaJHealth );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHaJHealth::CHudHaJHealth( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudHaJHealth")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	
	SetScheme("SourceScheme");

	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD | HIDEHUD_HEALTH );
	
	m_bShow = true;
	
	m_pBackground = NULL;
	m_pIconHealth = NULL;
	m_pIconHealthCrit = NULL;
	m_pIconSprint = NULL;
}

void CHudHaJHealth::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
}

void CHudHaJHealth::ShouldDraw( bool draw )
{	
	m_bShow = draw;
}

void CHudHaJHealth::Init( void )
{	
	Reset();
	VidInit();
	InvalidateLayout();
}

void CHudHaJHealth::Reset( void )
{
	// reset values
	m_iHealth = INIT_HEALTH;
	m_iStamina = -1;
	m_bitsDamage = 0;
	m_iTeammatesNearby = 0;
}

void CHudHaJHealth::VidInit( void )
{
	// load textures/icons
	m_pBackground =	gHUD.GetIcon( m_szBgTexture );
	m_pIconHealth = gHUD.GetIcon( "icon_health" );
	m_pIconHealthCrit = gHUD.GetIcon( "icon_health_crit" );
	m_pIconSprint = gHUD.GetIcon( "icon_sprint" );
	m_pIconTeamwork = gHUD.GetIcon( "icon_teamwork" );

}

void CHudHaJHealth::OnThink( void )
{
	UpdateDisplay();
}

//-----------------------------------------------------------------------------
// Purpose: organises the panel
//-----------------------------------------------------------------------------
void CHudHaJHealth::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Draws the panel
//-----------------------------------------------------------------------------
void CHudHaJHealth::Paint()
{
	// check if we should draw
	if ( !m_bShow )
		return;

	// draw background
	if ( m_pBackground != NULL )
		m_pBackground->DrawSelf( 0, 0, GetWide(), GetTall(), Color( 255,255,255, 255 ) );

	surface()->DrawSetTextColor( Color( 0, 0, 0, 255) );

	// draw icons
	if ( m_iHealth <= 10 )
	{
		if ( m_pIconHealthCrit != NULL )
			m_pIconHealthCrit->DrawSelf( m_flHealthIconX, m_flHealthIconY, m_flIconWidth, m_flIconHeight, Color(255, 255, 255, 255) );
	}
	else
	{
		if ( m_pIconHealth != NULL )
			m_pIconHealth->DrawSelf( m_flHealthIconX, m_flHealthIconY, m_flIconWidth, m_flIconHeight, Color(255, 255, 255, 255) );
	}

	if ( m_pIconSprint != NULL )
		m_pIconSprint->DrawSelf( m_flSprintIconX, m_flSprintIconY, m_flIconWidth, m_flIconHeight, Color(255,255,255, 255) );

	if( m_pIconTeamwork != NULL )
		m_pIconTeamwork->DrawSelf( m_flTeamIconX, m_flTeamIconY, m_flIconWidth, m_flIconHeight, Color( 255, 255, 255, 255 ) );

	int enabledChunks = 0;
	int i, xpos, ypos;

	// draw health bar
	m_flBarChunkWidth = m_flBarWidth / HEALTH_CHUNKS;
	enabledChunks = 0;
	
	// calculate the number of chunks to show for the stamina bar
	if ( m_iHealth > 0 )
		enabledChunks = ( m_iHealth / ( 100 / ( HEALTH_CHUNKS - 1 ) ) ) + 1;
/*
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	engine->Con_NPrintf( 0, "Health: %d", m_iHealth );
	engine->Con_NPrintf( 1, "Chunks: %d", enabledChunks );
#endif	
*/
	// draw
	surface()->DrawSetColor( m_HealthLeftColor );
	xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	
	for (i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}
	
	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( m_HealthGoneColor );
		
	for (i = enabledChunks; i < HEALTH_CHUNKS; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}

	//
	// draw the stamina bar
	//
	m_flSBarChunkWidth = m_flSBarWidth / STAMINA_CHUNKS;
	enabledChunks = 0;
	
	// calculate the number of chunks to show for the stamina bar
	if ( m_iStamina > 0 )
		enabledChunks = ( m_iStamina / ( 100 / ( STAMINA_CHUNKS - 1 ) ) ) + 1;
/*
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	engine->Con_NPrintf( 2, "Stamina: %d", m_iStamina );
	engine->Con_NPrintf( 3, "Chunks: %d", enabledChunks );
#endif	
*/	
	// draw
	surface()->DrawSetColor( m_StaminaLeftColor );
	xpos = m_flSBarInsetX, ypos = m_flSBarInsetY;
	
	for (i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flSBarChunkGap ), ypos + m_flSBarHeight );
		xpos += m_flBarChunkWidth;
	}
	
	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( m_StaminaGoneColor );
		
	for (i = enabledChunks; i < STAMINA_CHUNKS; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flSBarChunkGap ), ypos + m_flSBarHeight );
		xpos += m_flBarChunkWidth;
	}

	//
	// draw team mates nearby bar
	//
	m_flTBarChunkWidth = m_flTBarWidth / TEAM_CHUNKS;
	enabledChunks = 0;	

	// calculate the number of chunks to show for the teamwork bar
	if ( m_iTeammatesNearby > 0 )
		enabledChunks = m_iTeammatesNearby;

	// draw
	surface()->DrawSetColor( m_TeamLeftColor );
	xpos = m_flTBarInsetX, ypos = m_flTBarInsetY;

	for (i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flTBarChunkWidth * m_flTBarChunkGap ), ypos + m_flTBarHeight );
		xpos += m_flTBarChunkWidth;
	}

	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( m_TeamGoneColor );

	for (i = enabledChunks; i < TEAM_CHUNKS; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + (m_flTBarChunkWidth * m_flTBarChunkGap ), ypos + m_flTBarHeight );
		xpos += m_flTBarChunkWidth;
	}


}

//-----------------------------------------------------------------------------
// Purpose: organises the panel
//-----------------------------------------------------------------------------
void CHudHaJHealth::UpdateDisplay()
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

	if ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return;

	if ( pLocalPlayer->GetCurrentClass() == CLASS_UNASSIGNED )
		return;

	if ( pLocalPlayer->IsAlive() == false )
		 return;

	UpdatePlayerStatus( pLocalPlayer );
}

void CHudHaJHealth::UpdatePlayerStatus( C_HL2MP_Player *player )
{
	// make our panel drawable.
	SetVisible ( true );
	ShouldDraw ( true );

	int newHealth = 0;
	int newStamina = 0;
	
	newHealth = max( player->GetHealth(), 0 );
	newStamina = max( player->m_HL2Local.m_flSuitPower, 0 );

	if ( newHealth != m_iHealth )
		m_iHealth = newHealth;
		
	if ( newStamina != m_iStamina )
		m_iStamina = newStamina;

	C_HajPlayer* pPlayer = ToHajPlayer( player );

	if( pPlayer)
		m_iTeammatesNearby = clamp( pPlayer->GetNearbyTeammates(), 0, TEAM_CHUNKS );
	else
		m_iTeammatesNearby = 0;

}