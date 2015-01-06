//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose: HUD element which shows whether you can vault
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_playerresource.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudVaultHint : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudVaultHint, vgui::Panel );

public:
	CHudVaultHint( const char *pElementName );

	void VidInit( void );

protected:
	virtual void OnThink();
	virtual void Paint( void );

private:

	CHudTexture		*m_pBackground;		// background texture
	CHudTexture		*m_pBackgroundVault;
	CHudTexture		*m_pBackgroundCantVault;

	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_vault" );
	CPanelAnimationStringVar( 128, m_szNoVaultTexture, "NoVaultBackground", "hud_novault" );

};

DECLARE_HUDELEMENT( CHudVaultHint );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudVaultHint::CHudVaultHint( const char *pElementName ) : BaseClass(NULL, "HudVaultHint"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD | HIDEHUD_HEALTH );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled( false );
	SetVisible(false);

}
void CHudVaultHint::VidInit( void )
{
	m_pBackgroundVault = gHUD.GetIcon( m_szBgTexture );
	m_pBackgroundCantVault = gHUD.GetIcon( m_szNoVaultTexture );
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Mg info from the weapon
//-----------------------------------------------------------------------------
extern ConVar haj_player_allow_vault;
void CHudVaultHint::OnThink()
{
	BaseClass::OnThink();

	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( pPlayer && haj_player_allow_vault.GetBool() )
	{
		SetVisible( ( pPlayer->m_Local.m_bVaultingDisabled || pPlayer->CanVault() ) );

		if( pPlayer->m_Local.m_bVaultingDisabled )
			m_pBackground = m_pBackgroundCantVault;
		else
			m_pBackground = m_pBackgroundVault;

		return;
	}

	SetVisible( false );

}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudVaultHint::Paint( void )
{
	BaseClass::Paint();

	if ( m_pBackground != NULL )
		m_pBackground->DrawSelf( 0, 0, GetWide(), GetTall(), Color(255, 255, 255, 255) );

}
