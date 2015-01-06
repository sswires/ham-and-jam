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
class CHudAddedTime : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudAddedTime, vgui::Panel );

public:
	CHudAddedTime( const char *pElementName );

	void Init( void );
	//void VidInit( void );
	void FireGameEvent( IGameEvent * event );

protected:
	void OnThink( void );
	void Paint( void );

private:
	float m_flTimeAdded;
	float m_flTimeOfNotification;
	wchar_t m_wszFormattedTimeAdd[75];

	CPanelAnimationVar( float, m_flFadeTime, "FadeTime", "5.0" );
	CPanelAnimationVar( float, m_flFadeLength, "FadeLength", "1.02" );
	CPanelAnimationVar( float, m_flHighlightTime, "HighlightTime", "0.25" );
	CPanelAnimationVar( Color, m_AddedColor, "AddedTimeColor", "0 200 0 255" );
	CPanelAnimationVar( Color, m_ReducedColor, "ReducedTimeColor", "200 0 0 255" );
	CPanelAnimationVar( int, m_iColorAddHighlight, "ColorAddHighlight", "50" );
	CPanelAnimationVar( vgui::HFont, m_hTimeFont, "AddedTimeFont", "TimeFont" );

};

DECLARE_HUDELEMENT( CHudAddedTime );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAddedTime::CHudAddedTime( const char *pElementName ) : BaseClass(NULL, "HudAddedTime"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
	SetVisible( true );
	SetProportional( true );

	m_flTimeAdded = 0.0f;
	m_flTimeOfNotification = -1.0f;

}

// Adds event listener for time added
void CHudAddedTime::Init( void )
{
	gameeventmanager->AddListener( this, "roundtime_extend", false );

	m_flTimeOfNotification = -1.0f;
}

// called when event that we're listening to is fired, which will be roundtime_extend
void CHudAddedTime::FireGameEvent( IGameEvent * event )
{
	m_flTimeAdded = event->GetFloat( "time_added" );

	if( m_flTimeAdded != 0.0f )
	{
		m_flTimeOfNotification = gpGlobals->curtime;

		int iMinutes = m_flTimeAdded / 60;
		int iSeconds = (int)m_flTimeAdded % 60;

		char szSign[2];

		if( m_flTimeAdded >= 0.0f )
			szSign[0] = '+';
		else
			szSign[0] = '-';

		szSign[1] = '\0';

		_snwprintf( m_wszFormattedTimeAdd, sizeof( m_wszFormattedTimeAdd ), L"%S %d:%02d", szSign, iMinutes, iSeconds );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAddedTime::Paint( void )
{
	if( m_flTimeOfNotification < 0.0f ) return; // if <0, it'll be an invalid time

	float flTimeElapsed = gpGlobals->curtime - m_flTimeOfNotification;
	Color drawCol = ( m_flTimeAdded > 0 ? m_AddedColor : m_ReducedColor );

	if( flTimeElapsed < 0 )
		return;

	if( flTimeElapsed < m_flHighlightTime ) // highlight
	{
		int coladd = ( ( m_flHighlightTime - flTimeElapsed ) / m_flHighlightTime ) * m_iColorAddHighlight;

		drawCol[0] += coladd;
		drawCol[1] += coladd;
		drawCol[2] += coladd;
	}

	// alpha fade
	if( flTimeElapsed >= m_flFadeTime - m_flFadeLength )
	{
		float over = flTimeElapsed - ( m_flFadeTime - m_flFadeLength );
		float fraction = ( over / m_flFadeLength );

		drawCol[3] = ( 1.0f - fraction ) * 255;

		if( drawCol[3] < 1 )
		{
			m_flTimeOfNotification = -1.0f;
			return;
		}
	}

	int x, y, w, h;
	GetBounds( x, y, w, h );

	surface()->DrawSetTextColor( drawCol );

	// get the text size, to calc the position
	int tw, th;
	surface()->GetTextSize( m_hTimeFont, m_wszFormattedTimeAdd, tw, th );

	// draw
	surface()->DrawSetTextPos( ( w / 2 ) - ( tw / 2 ), ( h / 2 ) - ( th / 2 ) ); // centered
	surface()->DrawSetTextFont( m_hTimeFont );
	surface()->DrawPrintText( m_wszFormattedTimeAdd, Q_wcslen( m_wszFormattedTimeAdd ) );

}

void CHudAddedTime::OnThink( void )
{
	float flTimeElapsed = gpGlobals->curtime - m_flTimeOfNotification;

	if( m_flTimeOfNotification > -1.0f && flTimeElapsed > m_flFadeTime )
	{
		m_flTimeOfNotification = -1.0f;
	}
}