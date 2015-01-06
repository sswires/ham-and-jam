//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Hud panel which displays whether you can deploy the machinegun or not
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "haj_weapon_base.h"
#include "hudelement.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define MGHINT_UPDATE_RATE 0.6f

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudHaJMgHint : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHaJMgHint, vgui::Panel );

public:
	CHudHaJMgHint( const char *pElementName );

	void VidInit( void );

protected:
	virtual void OnThink();
	virtual void Paint( void );

	float m_flUpdatetime;

	
	
private:

	CHudTexture		*m_pBackground;		// background texture
	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_deployicon" );
};

DECLARE_HUDELEMENT( CHudHaJMgHint );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHaJMgHint::CHudHaJMgHint( const char *pElementName ) : BaseClass(NULL, "HudHajMg"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS | HIDEHUD_PLAYERDEAD | HIDEHUD_HEALTH );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
   SetParent( pParent ); 

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled( false );
	SetVisible(false);

}
void CHudHaJMgHint::VidInit( void )
{
	m_pBackground =	gHUD.GetIcon( m_szBgTexture );
}
//-----------------------------------------------------------------------------
// Purpose: called every frame to get Mg info from the weapon
//-----------------------------------------------------------------------------
void CHudHaJMgHint::OnThink()
{
	BaseClass::OnThink();

	CHAJWeaponBase *wpn = dynamic_cast< CHAJWeaponBase* >( GetActiveWeapon() );

	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();


	if (!pLocalPlayer || !wpn || !wpn->HasBipod() )
	{
		SetVisible(false);
		return;
	}
		
	// If we have the bipod deployed no need to hide the icon
	if ( wpn->m_bIsBipodDeployed )
		SetVisible(true);
	else
		SetVisible( wpn->CanDeployBipod( false ) );

}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHaJMgHint::Paint( void )
{
	BaseClass::Paint();

	if ( m_pBackground != NULL )
		m_pBackground->DrawSelf( 0, 0, GetWide(), GetTall(), Color(255, 255, 255, 255) );

}
