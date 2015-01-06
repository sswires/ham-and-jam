//========= Copyright © 2010, Ham and Jam. ==============================//
//
// Purpose: HUD element that shows you when time is added or decreased from
//			the round
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "c_playerresource.h"

#include "haj_gamerules.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudRoundCountdown : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudRoundCountdown, vgui::Panel );

public:
	CHudRoundCountdown( const char *pElementName );

	void Init( void );
	//void VidInit( void );
	//void FireGameEvent( IGameEvent * event );

protected:
	void OnThink( void );
	void Paint( void );
	bool ShouldDraw( void );

private:

	CPanelAnimationVar( vgui::HFont, m_hTimeFont, "CountdownSecondsFont", "TimeFont" );
	CPanelAnimationVar( vgui::HFont, m_hRoundBeginFont, "RoundBeginsInFont", "GeneralFont" );
	CPanelAnimationVarAliasType( int, m_iFreePlayYOffset, "FreeplayYOffset", "25", "proportional_int" );

};

DECLARE_HUDELEMENT_DEPTH( CHudRoundCountdown, 1 );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudRoundCountdown::CHudRoundCountdown( const char *pElementName ) : BaseClass(NULL, "HudRoundCountdown"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
	SetVisible( true );
	SetProportional( true );
}

// Adds event listener for time added
void CHudRoundCountdown::Init( void )
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
extern ConVar haj_freezetime;
extern ConVar haj_freeplaytime;
void CHudRoundCountdown::Paint( void )
{
	CHajGameRules *pGameRules = HajGameRules();

	if( pGameRules )
	{
		float flCountdown; 
		
		if( pGameRules->IsFreezeTime() )
			flCountdown = clamp( haj_freezetime.GetFloat() - ( gpGlobals->curtime - pGameRules->GetRoundStartTime() ) + 1, 0, haj_freezetime.GetFloat() );
		else if( pGameRules->IsFreeplay() )
			flCountdown = haj_freeplaytime.GetInt() - gpGlobals->curtime;
		else
			return;

		int x, y, w, h;
		GetBounds( x, y, w, h );

		int y_offset = 0;

		wchar_t wszDescription[125];
		_snwprintf( wszDescription, sizeof( wszDescription ), L"%s", ( pGameRules->IsFreezeTime() ? localize()->Find( "#HaJ_Timer_Freezetime" ) : localize()->Find( "#HaJ_Timer_Freeplay" ) ) );

		if( pGameRules->IsFreeplay() )
			y_offset += m_iFreePlayYOffset; 

		// get the text size, to calc the position
		int twd, thd;
		surface()->GetTextSize( m_hRoundBeginFont, wszDescription, twd, thd );

		// draw
		surface()->DrawSetTextColor( 255, 255, 255, 255 );
		surface()->DrawSetTextPos( ( w / 2 ) - ( twd / 2 ), y_offset ); // centered
		surface()->DrawSetTextFont( m_hRoundBeginFont );
		surface()->DrawPrintText( wszDescription, Q_wcslen( wszDescription ) );

		// now to draw the time
		wchar_t wszSeconds[15];
		_snwprintf( wszSeconds, sizeof( wszSeconds ), L"%d", (int)( flCountdown ) );

		// get the text size, to calc the position
		int tw, th;
		surface()->GetTextSize( m_hTimeFont, wszSeconds, tw, th );

		float flScale = clamp( ( ( 0.5 - ( flCountdown - floor( flCountdown ) ) ) * 1.5 ), 1, 1.5 );

		// draw
		//surface()->DrawSetTextScale( flScale, flScale );
		surface()->DrawSetTextColor( 255, 255, ( 1 - ( flCountdown - floor( flCountdown ) ) ) * 255, 255 );
		surface()->DrawSetTextPos( ( w / 2 ) - ( tw / 2 ), thd + 10 + y_offset ); // centered
		surface()->DrawSetTextFont( m_hTimeFont );
		surface()->DrawPrintText( wszSeconds, Q_wcslen( wszSeconds ) );
	}
}

void CHudRoundCountdown::OnThink( void )
{

}

bool CHudRoundCountdown::ShouldDraw( void )
{
	CHajGameRules *pGameRules = HajGameRules();

	if( pGameRules )
		return ( pGameRules->IsFreezeTime() || pGameRules->IsFreeplay() );

	return false;
}