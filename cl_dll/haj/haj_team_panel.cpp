//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Team panel shows how many team mates are near you
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "haj_player_c.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"
#include "hl2mp_gamerules.h"
#include "c_team.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define POWER_CHUNKS	5

//-----------------------------------------------------------------------------
// Purpose: Shows how many team mates are near you
//-----------------------------------------------------------------------------
class CHudTeamPanel : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudTeamPanel, vgui::Panel );

public:
	CHudTeamPanel( const char *pElementName );
	
	virtual void	Init( void );
	virtual void	Reset( void );
	virtual void	OnThink( void );
	bool			ShouldDraw( void );

protected:
	virtual void	Paint();

private:
	// variables
	int				m_nTeammates;
	float			m_flBarChunkWidth;
	bool			NeedsRedraw();

	C_HajPlayer		*pPlayer;

	// RES variables

	// power colours
	CPanelAnimationVar( Color, m_PowerColor, "TeamColor", "255 255 0 192" );
	CPanelAnimationVar( Color, m_PowerColorDim,	"TeamDimColor", "128 128 0 192" );
	
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "14", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "10", "proportional_float" );
	CPanelAnimationVar( float, m_flBarChunkGap, "BarChunkGapRatio", "0.8" );
};

DECLARE_HUDELEMENT( CHudTeamPanel );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudTeamPanel::CHudTeamPanel( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudTeamPanel")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetScheme("ClientScheme");
	
	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD | HIDEHUD_HEALTH );
	
	m_nTeammates = 0.0f;
	pPlayer = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTeamPanel::Init( void )
{
	m_nTeammates = 0;
	pPlayer = C_HajPlayer::GetLocalHajPlayer();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTeamPanel::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudTeamPanel::ShouldDraw()
{
	C_HajPlayer* pTPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( pTPlayer && pTPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	return  CHudElement::ShouldDraw();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTeamPanel::OnThink( void )
{
	if( !pPlayer )
		pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if ( pPlayer && pPlayer->IsLocalPlayer() ) // crashing some clients?
	{
		m_nTeammates = clamp( pPlayer->GetNearbyTeammates(), 0, 5 );
		return;
	}

	m_nTeammates = 0;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTeamPanel::Paint( void )
{

	int chunkCount;
	int i, xpos, ypos;
	
	m_flBarChunkWidth = m_flBarWidth / POWER_CHUNKS;
	chunkCount = POWER_CHUNKS;

	surface()->DrawSetColor( m_PowerColor );
	xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	
	for (i = 0; i < m_nTeammates; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}

	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( m_PowerColorDim );
		
	for (i = m_nTeammates; i < chunkCount; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}
}