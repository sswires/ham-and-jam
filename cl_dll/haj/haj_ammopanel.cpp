//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Ammo status panel that shows ammo/magazines left
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

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/AnimationController.h>
#include "vgui_controls/BitmapImagePanel.h"
#include "vgui_bitmapimage.h"

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays current ammo and magazines left
//-----------------------------------------------------------------------------
class CHudHaJAmmo : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHaJAmmo, vgui::Panel );

public:
	CHudHaJAmmo( const char *pElementName );
	
	void Init( void );
	void VidInit( void );
	void ShouldDraw( bool draw );

	virtual void Reset( void );
	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void OnThink();
	virtual void Paint( void );

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo( C_HL2MP_Player *player );
	
private:
	
	// variables
	bool			m_bShow;			// show panel or not?
	int				m_iAmmo;			// ammo in weapon
	int				m_iMags;			// magaazines left
	bool			m_bShowClipBar;
	int				m_iChunks;			// how many chunks the ammo bar has
	int				m_iCurrentWeaponMaxAmmo;
	int				m_iEnabledChunks;

	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;

	// textures
	CHudTexture		*m_pTexMagazine;	// magazine texture
	BitmapImage		*m_pBackground;		// background texture

	// Panel variables.
	// Note: starting position X&Y are from the top-left and are set by default in the RES
	// so we don't need to re-define them here.

	// TODO: Once the default layout is decided, copy the values from the RES file to here.
	
	// background texture
	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_bg_generic" );
	CPanelAnimationVarAliasType( int, m_iBackgroundX, "BackgroundX", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBackgroundY, "BackgroundY", "0", "proportional_int" );
	CPanelAnimationVar( int, m_iAlignment, "TextAlign", "1" );

	// ammo bar
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "78", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "7", "proportional_float" );
	CPanelAnimationVarAliasType( int, m_iBarPadding, "BarPadding", "1", "proportional_int" );

	CPanelAnimationVar( float, m_flBarChunkGap, "BarChunkGapRatio", "0.9" );
	CPanelAnimationVar( Color, m_AmmoColor, "AmmoColor", "64 64 64 255" );
	CPanelAnimationVar( Color, m_AmmoColorDim,	"AmmoDimColor", "64 64 64 64" );
	CPanelAnimationVar( Color, m_MagsColor, "MagsColor", "255 255 255 255" );
	
	// mag image dimensions and pos
	CPanelAnimationVarAliasType( float, m_flImageWidth, "ImageWidth",  "48", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageHeight, "ImageHeight", "48", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageX, "ImageX",  "12", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageXNoClip, "ImageNoClipX", "12", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flImageY, "ImageY", "17", "proportional_float" );
	
	// mags left text
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "MagCount" );
	CPanelAnimationVarAliasType( float, m_flAmmoPosX, "AmmoPosX", "60", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flAmmoPosY, "AmmoPosY", "48", "proportional_float" );

};

DECLARE_HUDELEMENT( CHudHaJAmmo );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHaJAmmo::CHudHaJAmmo( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudHaJAmmo")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetScheme("ClientScheme");
	
	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD | HIDEHUD_HEALTH );
	
	m_bShow = true;
	m_iAmmo = 0;
	m_iMags = 0;
	m_pTexMagazine = NULL;
	m_pBackground = NULL;
	m_iEnabledChunks = 0;
	m_iCurrentWeaponMaxAmmo = 0;
	m_iChunks = 5;
	m_bShowClipBar = false;

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
}

void CHudHaJAmmo::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
}

void CHudHaJAmmo::ShouldDraw( bool draw )
{	
	m_bShow = draw;
}

void CHudHaJAmmo::Init( void )
{	
	Reset();
	InvalidateLayout();
}

void CHudHaJAmmo::Reset( void )
{
	m_iAmmo = 0;
	m_iMags = 0;
	m_pTexMagazine = NULL;
	m_iChunks = 5;
	m_iEnabledChunks = 0;
	m_bShowClipBar = false;
}

void CHudHaJAmmo::VidInit( void )
{
	m_pBackground =	new BitmapImage( GetVPanel(), m_szBgTexture );
}

void CHudHaJAmmo::OnThink( void )
{
	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: organises the panel
//-----------------------------------------------------------------------------
void CHudHaJAmmo::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Draws the panel
//-----------------------------------------------------------------------------
void CHudHaJAmmo::Paint()
{
	// check if we should draw
	if ( !m_bShow )
		return;

	if ( m_pBackground != NULL )
		m_pBackground->DoPaint( m_iBackgroundX, m_iBackgroundY, GetWide() - m_iBackgroundX, GetTall() - m_iBackgroundY );
	
	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( m_MagsColor );
	
	wchar_t mags[ 4 ];
	_snwprintf( mags, sizeof(mags), L"%i", m_iMags );

	//vgui::localize()->ConvertANSIToUnicode( mags2, mags, sizeof( mags ) );
	if( m_iAlignment == 1 )
	{
		int w, h;
		surface()->GetTextSize( m_hTextFont, mags, w, h );

		surface()->DrawSetTextPos( m_flAmmoPosX - (float)w, m_flAmmoPosY );
	}
	else
		surface()->DrawSetTextPos( m_flAmmoPosX, m_flAmmoPosY );

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawUnicodeString( mags );

	if ( m_bShowClipBar )
	{
		int xpos, ypos;
		xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	
		// draw the background
		surface()->DrawSetColor( m_AmmoColorDim );
		surface()->DrawFilledRect( xpos, ypos, xpos + m_flBarWidth, ypos + m_flBarHeight );

		// draw the bar which represents the ammo
		if( m_iCurrentWeaponMaxAmmo > 0 && m_iAmmo > 0 )
		{
			surface()->DrawSetColor( m_AmmoColor );

			float flAmmoFrac = (float)m_iAmmo / (float)m_iCurrentWeaponMaxAmmo;
			float flAmmoWidth = flAmmoFrac * m_flBarWidth;

			surface()->DrawFilledRect( xpos + m_iBarPadding, ypos + m_iBarPadding, xpos + flAmmoWidth - m_iBarPadding, ypos + m_flBarHeight - m_iBarPadding );
		}

	}

	if ( m_pTexMagazine != NULL )
	{
		surface()->DrawSetTexture( m_pTexMagazine->textureId );
		m_pTexMagazine->DrawSelf( m_bShowClipBar ? m_flImageX : m_flImageXNoClip, m_flImageY, m_flImageWidth, m_flImageHeight, Color( 255, 255, 255, 255) );
	}

}
//-----------------------------------------------------------------------------
// Purpose: organises the panel
//-----------------------------------------------------------------------------
void CHudHaJAmmo::UpdateAmmoDisplays()
{

	// disable the panel by default.
	ShouldDraw ( false );
	SetVisible ( false );

	if ( !CHudElement::ShouldDraw() )
		return;
	
	if ( engine->IsDrawingLoadingImage() )
		return;

	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	IClientVehicle *pVehicle = pLocalPlayer ? pLocalPlayer->GetVehicle() : NULL;

	// don't draw the panel if any of the below are true.
	if ( pLocalPlayer == NULL )
		 return;

	if ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return;

	if ( pLocalPlayer->IsAlive() == false )
		 return;

	if ( pVehicle )
		return;

	UpdatePlayerAmmo( pLocalPlayer );

}

void CHudHaJAmmo::UpdatePlayerAmmo( C_HL2MP_Player *player )
{
	// Clear out the vehicle entity
	m_hCurrentVehicle = NULL;

	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	
	if ( !wpn || !player || !wpn->UsesPrimaryAmmo() )
		return;
	
	// make our panel drawable.
	SetVisible ( true );
	ShouldDraw ( true );

	// get the correct image for this magazine
	m_pTexMagazine = (CHudTexture*)wpn->GetSpriteAmmo();
	
	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	int ammo2;

	(ammo1 < 0) ? m_bShowClipBar = false : m_bShowClipBar = true; 

	if ( !m_bShowClipBar )
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		ammo2 = ammo1;
		m_iEnabledChunks = 0;
	}
	else
	{
		int iClipSize = wpn->GetMaxClip1();
		m_iCurrentWeaponMaxAmmo = iClipSize;
		
		// we use clip ammo, so the second ammo is the mags left
		ammo2 = player->GetAmmoCount(wpn->GetPrimaryAmmoType()); //wpn->MagsLeft();
	}

	m_iAmmo = ammo1;
	m_iMags = ammo2;

	if (wpn != m_hCurrentActiveWeapon)
	{
		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			m_bShowClipBar = true;
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			m_bShowClipBar = false;
		}
		
		m_hCurrentActiveWeapon = wpn;
	}
};
