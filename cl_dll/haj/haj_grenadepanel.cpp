#include "cbase.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"
#include "hl2mp_gamerules.h"
#include "c_team.h"

#include "haj_weapon_grenadebase.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define POWER_CHUNKS	5

// add the classnames of grenades here
const char *g_ppszGrenadeWeapons[] = 
{
	"C_WeaponMills",
	"C_WeaponStiel",
	"C_WeaponSmokeNb39",
	"C_WeaponSmokeNo77",
};

// Cvars for powerbar
ConVar haj_cl_show_grenade_bar( "haj_cl_show_grenade_bar", "0", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Draw grenade timer bar?"); 

//-----------------------------------------------------------------------------
// Purpose: Displays how much "oomph" behind a grenade throw
//-----------------------------------------------------------------------------
class CHudGrenadePanel : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudGrenadePanel, vgui::Panel );

public:
	CHudGrenadePanel( const char *pElementName );
	
	virtual void	Init( void );
	virtual void	Reset( void );
	virtual void	OnThink( void );
	bool			ShouldDraw( void );

protected:
	virtual void	Paint();

private:
	// variables
	float			m_fPower;		//	Player Health
	float			m_flBarChunkWidth;
	bool			m_bIsGrenade;
	bool			IsGrenade();
	bool			NeedsRedraw();

	// RES variables

	// power colours
	CPanelAnimationVar( Color, m_PowerColor, "PowerColor", "255 255 255 192" );
	CPanelAnimationVar( Color, m_PowerColorDim,	"PowerDimColor", "128 128 128 192" );
	
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "14", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "96", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "10", "proportional_float" );
	CPanelAnimationVar( float, m_flBarChunkGap, "BarChunkGapRatio", "0.8" );
};

DECLARE_HUDELEMENT( CHudGrenadePanel );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenadePanel::CHudGrenadePanel( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudGrenadePanel")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetScheme("ClientScheme");
	
	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD );
	
	m_fPower = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenadePanel::Init( void )
{
	m_fPower = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenadePanel::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
extern ConVar haj_allow_grenade_bar;
bool CHudGrenadePanel::ShouldDraw()
{
	if( !haj_allow_grenade_bar.GetBool() || !haj_cl_show_grenade_bar.GetBool() )
		return false;

	bool bNeedsDraw = false;

	// get a handle to the player
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		 return false;

	// get a handle to their weapon
	C_WeaponGrenade *pWeapon = dynamic_cast< C_WeaponGrenade* >( pPlayer->GetActiveWeapon() );

	if ( pWeapon && pWeapon->CanCookGrenade() )
	{
		float flStartThrow = pWeapon->GetAttackTime();
		float flFuseTime = pWeapon->GetFuseTimer();

		float flEndTime = flStartThrow + flFuseTime;

		if( gpGlobals->curtime > flEndTime )
		{
			m_fPower = 0.0f;
			bNeedsDraw = false;
		}
		else
		{
			float flDiff = ( flEndTime - gpGlobals->curtime );
			m_fPower = ( flDiff / flFuseTime ) * 2.0f;
			m_fPower > 0.0f ? bNeedsDraw = true : bNeedsDraw = false;
		}
	}
	else
	{
		m_fPower = 0.0f;
		bNeedsDraw = false;
	}

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenadePanel::OnThink( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenadePanel::Paint( void )
{
	int chunkCount;
	int enabledChunks = 0;
	int i, xpos, ypos;
	
	m_flBarChunkWidth = m_flBarWidth / POWER_CHUNKS;
	chunkCount = POWER_CHUNKS;

	// calculate the number of chunks to show for the health bar
	if ( m_fPower > 0.0f )
	{
		if ( m_fPower == 2.0f )
			enabledChunks = 5;
		else if ( m_fPower > 1.5f )
			enabledChunks = 4;
		else if ( m_fPower > 1.1f )
			enabledChunks = 3;
		else if ( m_fPower > 0.7f )
			enabledChunks = 2;
		else if ( m_fPower > 0.0f )
			enabledChunks = 1;
		else
			enabledChunks = 0;
	}

	surface()->DrawSetColor( m_PowerColor );
	xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	
	for (i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}

	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( m_PowerColorDim );
		
	for (i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + ( m_flBarChunkWidth * m_flBarChunkGap ), ypos + m_flBarHeight );
		xpos += m_flBarChunkWidth;
	}
}