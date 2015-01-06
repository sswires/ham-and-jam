
// haj_hudtimer.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_hudtimer.h"
#include "haj_gamerules.h"
#include "c_hl2mp_player.h"

#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
DECLARE_HUDELEMENT(CHudTimer);

/////////////////////////////////////////////////////////////////////////////
CHudTimer::CHudTimer(const char* szElementName)
: CHudElement(szElementName)
, BaseClass(NULL, "HudTimer")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();

	SetParent(pParent);
	SetProportional(true);
	SetHiddenBits(HIDEHUD_HEALTH|HIDEHUD_PLAYERDEAD|HIDEHUD_NEEDSUIT);
}

/////////////////////////////////////////////////////////////////////////////
CHudTimer::~CHudTimer()
{

}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
	SetSize( 500, 50 );
}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::Init()
{
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(GetScheme());
	hFont = pScheme->GetFont("GeneralFont", true);
	hTimerFont = pScheme->GetFont("TimeFont", true );
}

extern ConVar haj_freeplaytime;
extern ConVar haj_freezetime;
float CHudTimer::GetTimeLeft()
{
	CHajGameRules* pGameRules = HajGameRules();
	
	if(!pGameRules)
		return 0.0f; // return 0 if gamerules isn't available

	float timeleft = pGameRules->GetRoundTimeLeft();

	/*if( pGameRules->IsFreezeTime())
		timeleft = clamp( haj_freezetime.GetFloat() - ( gpGlobals->curtime - pGameRules->GetRoundStartTime() ) + 1, 0, haj_freezetime.GetFloat() );
	else*/ if( timeleft <= 0 || pGameRules->IsInOvertime() )
		timeleft = 0;
	/*else if( pGameRules->IsFreeplay() )
		timeleft = haj_freeplaytime.GetInt() - gpGlobals->curtime;*/

	return timeleft;
}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::OnThink()
{
	CHajGameRules* pGameRules = HajGameRules();
	
	if(!pGameRules)
		return;

	float timeleft = GetTimeLeft();

	int nMinutes = timeleft / 60;
	int nSeconds = (int)timeleft % 60;

	char szText[150] = "";

	Q_snprintf(szText, sizeof( szText ), "%02d:%02d", nMinutes, nSeconds);
	
	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();

	/*if( pGameRules->IsFreeplay() )
	{
		_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"Free Play - Waiting for players" );
	}
	else if( pGameRules->IsFreezeTime() )
	{
		_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"Round is about to begin..." );
	}
	else*/ if( pLocalPlayer && ( pLocalPlayer->GetTeamNumber() == TEAM_CWEALTH || pLocalPlayer->GetTeamNumber() == TEAM_AXIS ) )
	{
		int remainingtime = (int)( pGameRules->GetSpawnTimeForTeam( pLocalPlayer->GetTeamNumber() ) - gpGlobals->curtime );

		if( remainingtime >= 0 )
			_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"Reinforcements in %d seconds", remainingtime );
		else if( pGameRules->IsInOvertime() )
		{
			if( pGameRules->IsSuddenDeath() )
				_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"Sudden Death - No reinforcements" );
			else
				_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"Overtime - No reinforcements" );
		}
		else
			_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"" );
	}
	else
		_snwprintf (m_szReinforcements, sizeof( m_szReinforcements ), L"" );

	// convert from ASCII to UNICODE
	vgui::localize()->ConvertANSIToUnicode(szText, m_szTimeLeft, sizeof(m_szTimeLeft));

	m_lenTimeLeft = wcslen(m_szTimeLeft);
}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::Paint()
{
	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();

	// get the render surface
	vgui::ISurface* pSurface = vgui::surface();
	assert(NULL != pSurface);

	if( !hFont ) return;

	if( m_iDrawTime >= 1 )
	{
		pSurface->DrawSetTextFont(hTimerFont);
		pSurface->DrawSetTextColor(255, 255, 255, 255);

		pSurface->DrawSetTextPos( 0, 0 );
		pSurface->DrawPrintText(m_szTimeLeft, m_lenTimeLeft);
	}

	if( !pLocalPlayer || pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return;

	pSurface->DrawSetTextFont(hFont);

	int rlw, rlh;
	pSurface->GetTextSize( hFont, m_szReinforcements, rlw, rlh );

	pSurface->DrawSetTextPos( m_iReinforcementPosX, 0 );
	pSurface->DrawSetTextColor(255, 255, 255, 150);
	pSurface->DrawPrintText(m_szReinforcements, wcslen( m_szReinforcements ));

}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::Reset()
{

}

/////////////////////////////////////////////////////////////////////////////
void CHudTimer::VidInit()
{
	
}

bool CHudTimer::ShouldDraw()
{
	CHajGameRules* pGameRules = HajGameRules();

	if(!pGameRules)
		return false;

	return ( !(pGameRules->IsFreeplay()||pGameRules->IsGameOver()) );
}
/////////////////////////////////////////////////////////////////////////////
