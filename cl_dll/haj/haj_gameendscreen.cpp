//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose: HUD element which shows whether you can vault
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

#include "haj_gamerules.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: End of game info
//-----------------------------------------------------------------------------
class CHudEndGameScreen : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudEndGameScreen, vgui::Panel );

public:
	CHudEndGameScreen( const char *pElementName );

	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void	Init();

	virtual void	FireGameEvent( IGameEvent * event );

	void			UpdateTimeLeft( void );

protected:
	virtual void OnThink();
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

private:

	float m_flGameEndTime;
	vgui::Label *m_pNextMapName;
	vgui::Label *m_pNextMapTime;

	CPanelAnimationVar( vgui::HFont, m_hNextMapFont, "NextMapFont", "GeneralFont" );
	CPanelAnimationVar( vgui::HFont, m_hTimeFont, "TimeFont", "CENotificationLatest" );

};

DECLARE_HUDELEMENT( CHudEndGameScreen );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudEndGameScreen::CHudEndGameScreen( const char *pElementName ) : BaseClass(NULL, "HudEndGameScreen"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled( false );
	SetProportional( true );

	m_flGameEndTime = -1.0f;

	m_pNextMapName = new vgui::Label( this, "NextMapName", L"" );
	m_pNextMapTime = new vgui::Label( this, "NextMapTime", L"" );
}

void CHudEndGameScreen::Init()
{
	gameeventmanager->AddListener(this, "game_end", false );
}

void CHudEndGameScreen::FireGameEvent( IGameEvent * event )
{
	m_flGameEndTime = gpGlobals->curtime;

	// get the next map name
	wchar_t nextmap[200];
	wchar_t mapname[50];

	_snwprintf( mapname, sizeof( mapname ), L"%S", event->GetString( "next_map", "N/A" ) );
	vgui::localize()->ConstructString( nextmap, sizeof( nextmap ), vgui::localize()->Find( "#HaJ_EndGameScreen_NextMap" ), 1, vgui::localize()->Find( event->GetString( "next_map", "N/A" ) ) );

	m_pNextMapName->SetText( nextmap );
	m_pNextMapName->SizeToContents();
	m_pNextMapName->SetPos( GetWide() - YRES(5) - m_pNextMapName->GetWide(), GetTall() - YRES(20) );
}

void CHudEndGameScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetWide( ScreenWidth() );
	SetTall( ScreenHeight() );

	m_pNextMapName->SetFont( m_hNextMapFont );
	m_pNextMapTime->SetFont( m_hTimeFont );
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Mg info from the weapon
//-----------------------------------------------------------------------------
void CHudEndGameScreen::OnThink()
{
	UpdateTimeLeft();

	BaseClass::OnThink();
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
extern ConVar haj_intermission_length;
void CHudEndGameScreen::Paint( void )
{
	BaseClass::Paint();
}



void CHudEndGameScreen::UpdateTimeLeft( void )
{
	// print time left
	wchar_t timer[30];
	float timeleft = clamp( haj_intermission_length.GetFloat() - ( gpGlobals->curtime - m_flGameEndTime ), 0, haj_intermission_length.GetFloat() );

	int iMinutes = timeleft / 60;
	int iSeconds = (int)timeleft % 60;

	_snwprintf( timer, sizeof( timer ), L"%02d:%02d", iMinutes, iSeconds);

	m_pNextMapTime->SetText( timer );
	m_pNextMapTime->SizeToContents();
	m_pNextMapTime->SetPos( GetWide() - m_pNextMapTime->GetWide() - YRES(5), GetTall() - m_pNextMapName->GetTall() - m_pNextMapTime->GetTall() - YRES(10) );
}

bool CHudEndGameScreen::ShouldDraw( void )
{
	if( HajGameRules() && HajGameRules()->IsGameOver() && gpGlobals->curtime >= m_flGameEndTime )
		return true;

	return false;
}