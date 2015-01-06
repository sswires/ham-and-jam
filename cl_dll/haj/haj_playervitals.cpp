//========= Copyright © 2011, Ham and Jam. ==============================//
// Purpose: HUD element which shows important information to the
//			player
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_playerresource.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

#include "hudelement.h"
#include "hud_basetimer.h"
#include "vgui_BitmapImage.h"

#include "haj_gamerules.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Player info
//-----------------------------------------------------------------------------
class CHudPlayerVitals : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudPlayerVitals, vgui::Panel );

public:
	CHudPlayerVitals( const char *pElementName );

	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void	VidInit();

	//virtual void	FireGameEvent( IGameEvent * event );

	void			UpdateTimeLeft( void );

protected:
	virtual void	OnThink();
	virtual void	Paint( void );
	virtual bool	ShouldDraw( void );

	virtual void	PaintProgressGradient( float fFrac, int xpos, int ypos );

	void			PaintHealthBar( void );
	void			PaintStaminaBar( void );
	void			PaintMorale( void );

private:

	BitmapImage*	m_pHealthBar;
	BitmapImage*	m_pStaminaBar;
	BitmapImage*	m_pProgressGradient;
	BitmapImage*	m_pProgressBackground;

	CHudTexture*	m_pMoraleChevron;

	int				m_iPlayerHealth;
	int				m_iPlayerStamina;
	int				m_iPlayerMorale;

	float			m_flLastMorale[3];

	CPanelAnimationVarAliasType( int, m_iHealthYPos, "Health_YPos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iStaminaYPos, "Stamina_YPos", "32", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarPosX, "Bar_StartPosX", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarWidth, "Bar_Width", "75", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarHeight, "Bar_Height", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarPadding, "Bar_Padding", "3", "proportional_int" );

	CPanelAnimationVar( Color, m_HealthGradiantBar, "Health_GradColor", "200 0 0 255" );
	CPanelAnimationVar( Color, m_StaminaGradiantBar, "Stamina_GradColor", "0 200 0 255" );

	CPanelAnimationVarAliasType( float, m_flMoraleChunkWidth, "MoraleChunkWidth", "25", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flMoraleChunkHeight, "MoraleChunkHeight", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flMoraleChunkSpacing, "MoraleChunkSpacing", "3", "proportional_float" );
	CPanelAnimationVarAliasType( int, m_iMoraleX, "MoraleX", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iMoraleY, "MoraleY", "0", "proportional_int" );

};

DECLARE_HUDELEMENT( CHudPlayerVitals );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPlayerVitals::CHudPlayerVitals( const char *pElementName ) : BaseClass(NULL, "HudPlayerVitals"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled( false );
	SetProportional( true );

	m_pHealthBar = new BitmapImage( GetVPanel(), "hud/hud_health" );
	m_pStaminaBar = new BitmapImage( GetVPanel(), "hud/hud_stam" );
	//m_pMoraleImage = new BitmapImage( GetVPanel(), "hud/hud_morale" );

	m_pProgressGradient = new BitmapImage( GetVPanel(), "hud/hud_bar_fill" );
	m_pProgressBackground = new BitmapImage( GetVPanel(), "hud/hud_bar_bg" );

	m_pMoraleChevron = NULL;

	for( int i = 0; i < 3; i++ )
	{
		m_flLastMorale[i] = 0.0f;
	}
}



void CHudPlayerVitals::VidInit()
{
	m_pMoraleChevron = gHUD.GetIcon( "hud_morale" );
}

void CHudPlayerVitals::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame, get player stats
//-----------------------------------------------------------------------------
void CHudPlayerVitals::OnThink()
{
	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();
	C_HajPlayer *pRealLocalPlayer = pLocalPlayer;

	if( pLocalPlayer && pLocalPlayer->IsObserver() && pLocalPlayer->GetObserverTarget() )
	{
		pLocalPlayer = ToHajPlayer( pLocalPlayer->GetObserverTarget() );
	}

	if( pLocalPlayer && pLocalPlayer->IsAlive() && !pLocalPlayer->IsObserver() )
	{
		m_iPlayerHealth = pLocalPlayer->GetHealth();
		m_iPlayerMorale = pLocalPlayer->GetNearbyTeammates();

		if( pLocalPlayer == pRealLocalPlayer )
			m_iPlayerStamina = (int)pLocalPlayer->m_HL2Local.m_flSuitPower;
		else
			m_iPlayerStamina = -1;
	}
	else
	{
		m_iPlayerHealth = -1;
		m_iPlayerStamina = -1;
	}

	BaseClass::OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: Show it
//-----------------------------------------------------------------------------
void CHudPlayerVitals::Paint( void )
{
	if( m_iPlayerHealth < 0 )
		return;

	PaintMorale();

	PaintHealthBar();
	PaintStaminaBar();

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Should we show it
//-----------------------------------------------------------------------------
bool CHudPlayerVitals::ShouldDraw( void )
{
	CHajGameRules* pGameRules = HajGameRules();

	if(!pGameRules)
		return false;

	return ( !(pGameRules->IsGameOver()) && CHudElement::ShouldDraw() );
}

void CHudPlayerVitals::PaintHealthBar( void )
{
	m_pProgressGradient->SetColor( m_HealthGradiantBar );
	PaintProgressGradient( clamp((float)(m_iPlayerHealth)/100, 0, 100), m_iBarPosX, m_iHealthYPos );

	m_pHealthBar->DoPaint( 0, m_iHealthYPos, GetWide(), m_iBarHeight );
}

void CHudPlayerVitals::PaintStaminaBar( void )
{
	if( m_iPlayerStamina < 0 )
		return;

	m_pProgressGradient->SetColor( m_StaminaGradiantBar );
	PaintProgressGradient( clamp((float)(m_iPlayerStamina)/100, 0, 100), m_iBarPosX, m_iStaminaYPos );

	m_pStaminaBar->DoPaint( 0, m_iStaminaYPos, GetWide(), m_iBarHeight );

}

void CHudPlayerVitals::PaintProgressGradient( float fFrac, int xpos, int ypos )
{
	m_pProgressBackground->DoPaint( xpos, m_iBarPadding + ypos, m_iBarWidth, m_iBarHeight - (m_iBarPadding*2) );

	if( fFrac > 0 )
	{
		m_pProgressGradient->DoPaint( xpos, m_iBarPadding + ypos, fFrac * m_iBarWidth, m_iBarHeight - (m_iBarPadding*2) );
	}
}

void CHudPlayerVitals::PaintMorale( void )
{
	if( m_pMoraleChevron != NULL && m_iPlayerMorale > 0 )
	{
		int x = m_iMoraleX;

		// draw chevrons, also have fade
		for( int i = 0; (i < m_iPlayerMorale || ( gpGlobals->curtime + 1.0f >= m_flLastMorale[i] && gpGlobals->curtime - m_flLastMorale[i] < 0.25f) ); i++ )
		{
			float flDrawAlpha = 1.0;

			if( i < m_iPlayerMorale )
				m_flLastMorale[i] = gpGlobals->curtime;
			else
				flDrawAlpha = 1.0f - ( ( gpGlobals->curtime - m_flLastMorale[i] ) * 4.0f );

			m_pMoraleChevron->DrawSelf( x, m_iMoraleY, m_flMoraleChunkWidth, m_flMoraleChunkHeight, Color( 255, 255, 255, (float)(flDrawAlpha * 255) ) );
			x += m_flMoraleChunkWidth + m_flMoraleChunkSpacing;
		}
	}
}