//========= Copyright © 2010, Ham and Jam. ==============================//
//
// Purpose: HUD element that displays notifcations to the player, at the
//			bottom center of the screen
//
// Author:  Stephen Swires
//
// $NoKeywords: $
//=======================================================================//


#include "cbase.h"
#include "iclientmode.h"
#include "hud_macros.h"

#include "c_playerresource.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

struct notification_t
{
	wchar_t wszLocalizedString[150];
	int iNotificationType;

	int iEntityID;
	wchar_t wszEntityName[90];
	wchar_t wszPlayerName[90];

	int iCEGain;

	float flExpireTime;
	float flCreateTime;
	int iDrawAlpha;

};

ConVar haj_cl_draw_notifications( "haj_cl_draw_notifications", "1", FCVAR_ARCHIVE );
ConVar haj_cl_notifications_time( "haj_cl_notifications_time", "4", FCVAR_ARCHIVE );

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudPlayerNotifications : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudPlayerNotifications, vgui::Panel );

public:
	CHudPlayerNotifications( const char *pElementName );

	void Init( void );
	//void VidInit( void );
	void FireGameEvent( IGameEvent * event );
	void MsgFunc_HajPlayerNotification( bf_read &msg );

protected:
	void OnThink( void );
	void Paint( void );

	CUtlVector< notification_t > m_Notifications;

private:

	CPanelAnimationVar( float, m_flFadeInTime, "FadeInTime", "0.1" );
	CPanelAnimationVar( float, m_flFadeTime, "FadeTime", "0.75" );
	CPanelAnimationVar( vgui::HFont, m_LatestFont, "LatestFont", "CaptureTitle" );
	CPanelAnimationVar( vgui::HFont, m_OldFont, "OlderFont", "GeneralFont" );
	CPanelAnimationVar( float, m_flStartAlpha, "StartAlpha", "150");

};

DECLARE_HUDELEMENT( CHudPlayerNotifications );
DECLARE_HUD_MESSAGE( CHudPlayerNotifications, HajPlayerNotification );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPlayerNotifications::CHudPlayerNotifications( const char *pElementName ) : BaseClass(NULL, "HudPlayerNotifications"), CHudElement( pElementName )
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
void CHudPlayerNotifications::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerNotifications, HajPlayerNotification );
}

/*
		HajPlayerNotification
		===================================
		(string) localized string to use for notifcation
		(int)    notification type (0 = just string, 1 = mention player, 2 = mention cap)
		
		if notification type > 0
			(int)		entity id of above
			(string)	name

		(int)	ce gain
		

*/
void CHudPlayerNotifications::MsgFunc_HajPlayerNotification( bf_read &msg )
{
	char localise[150];
	char entName[50], playerName[50];
	int entityId = 0;

	msg.ReadString( localise, sizeof( localise ) );
	
	int notificationType = msg.ReadShort();

	if( notificationType > 0 ) // not just a string, we got a player name or cap
	{
		entityId = msg.ReadShort();
		msg.ReadString( entName, sizeof( entName ) ); 

		C_BaseEntity *ent = C_BaseEntity::Instance( entityId );

		if( ent && ent->IsPlayer() && notificationType == 2 )
		{
			Q_snprintf( playerName, sizeof( playerName ), "%s", ToBasePlayer(ent)->GetPlayerName() );
		}
	}

	int ceGain = msg.ReadShort();

	notification_t notification;
	notification.flExpireTime = gpGlobals->curtime + haj_cl_notifications_time.GetFloat();
	notification.flCreateTime = gpGlobals->curtime;
	notification.iDrawAlpha = 0;
	notification.iCEGain = ceGain;
	notification.iNotificationType = notificationType;
	notification.iEntityID = entityId;
	
	_snwprintf( notification.wszEntityName, sizeof( notification.wszEntityName ), L"%S", entName );
	_snwprintf( notification.wszPlayerName, sizeof( notification.wszPlayerName ), L"%S", playerName );

	wchar_t* localisewide = vgui::localize()->Find( localise );

	if( localisewide )
		vgui::localize()->ConstructString( notification.wszLocalizedString, sizeof( notification.wszLocalizedString ), localisewide, 2, notification.wszEntityName, notification.wszPlayerName );
	else
		_snwprintf( notification.wszLocalizedString, sizeof( notification.wszLocalizedString ), L"%S", localise );

	m_Notifications.AddToTail( notification );
}

// called when event that we're listening to is fired, which will be roundtime_extend
void CHudPlayerNotifications::FireGameEvent( IGameEvent * event )
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudPlayerNotifications::Paint( void )
{
	if( !haj_cl_draw_notifications.GetBool() )
		return;

	bool bDrawnLatest = false;
	int y = 0;

	if( !m_Notifications.IsValidIndex( 0 ) )
		return;

	notification_t *notify = &m_Notifications[0];

	if( notify )
	{
		vgui::HFont useFont = m_LatestFont;

		int w, h;
		int cew = 0; // length of ce string

		vgui::surface()->DrawSetTextFont( useFont );
		vgui::surface()->GetTextSize( useFont, notify->wszLocalizedString, w, h );

			
		if( notify->iCEGain != 0 )
		{
			wchar_t cestring[80];
			Color col = Color( 0, 200, 0, notify->iDrawAlpha );

			if( notify->iCEGain < 0 ) // negative ce
				col = Color( 200, 0, 0, notify->iDrawAlpha );

			wchar_t *celocal = vgui::localize()->Find( "#HaJ_CENotify" );
			wchar_t ceitoa[5];
			_snwprintf( ceitoa, sizeof( ceitoa ), L"%s%d", (notify->iCEGain >= 0) ? "+" : "", notify->iCEGain );

			if( celocal && celocal[0] != '\0' )
				vgui::localize()->ConstructString( cestring, sizeof( cestring ), celocal, 1, ceitoa );
			else
				vgui::localize()->ConstructString( cestring, sizeof( cestring ), L"CE string token not found", 0 );

			vgui::surface()->GetTextSize( useFont, cestring, cew, h );
			vgui::surface()->DrawSetTextPos( (GetWide()/2) - (w/2) - (cew/2), y );
			vgui::surface()->DrawSetTextColor( col );
			vgui::surface()->DrawUnicodeString( cestring );

		}
			
		vgui::surface()->DrawSetTextPos( (GetWide()/2) - (w/2) + (cew/2), y );
		vgui::surface()->DrawSetTextColor( 255, 255, 255, notify->iDrawAlpha );
		vgui::surface()->DrawUnicodeString( notify->wszLocalizedString );

		y += h + YRES( 5 );

	}
}

void CHudPlayerNotifications::OnThink( void )
{
	// check only the visible notification
	if( !m_Notifications.IsValidIndex( 0 ) )
		return;

	notification_t *check = &m_Notifications[0];

	// fade in
	if( gpGlobals->curtime < check->flExpireTime && check->iDrawAlpha < (int)m_flStartAlpha)
		check->iDrawAlpha += ((m_flStartAlpha / m_flFadeInTime) * gpGlobals->frametime);

	// expiring
	if( gpGlobals->curtime >= check->flExpireTime )
		check->iDrawAlpha -= ( ( m_flStartAlpha / m_flFadeTime )  * gpGlobals->frametime );

	// remove
	if( check->flCreateTime + m_flFadeInTime + 0.5f > gpGlobals->curtime && check->iDrawAlpha < 1 )
	{
		m_Notifications.Remove(0);

		if(m_Notifications.IsValidIndex(0))
		{
			notification_t *check = &m_Notifications[0];

			check->flCreateTime = gpGlobals->curtime;
			check->flExpireTime = gpGlobals->curtime + haj_cl_notifications_time.GetFloat();
		}
	}
}