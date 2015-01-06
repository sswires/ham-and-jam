//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "haj_hud_chat.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include "vgui/ILocalize.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_hl2mp_player.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"

#include <vgui/IScheme.h>
#include <cl_dll/iviewport.h>
#include <igameresources.h>

ConVar cl_showtextmsg( "cl_showtextmsg", "1", 0, "Enable/disable text messages printing on the screen." );

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
static char* ConvertCRtoNL( char *str )
{
	for ( char *ch = str; *ch != 0; ch++ )
		if ( *ch == '\r' )
			*ch = '\n';
	return str;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
static wchar_t* ConvertCRtoNL( wchar_t *str )
{
	for ( wchar_t *ch = str; *ch != 0; ch++ )
		if ( *ch == L'\r' )
			*ch = L'\n';
	return str;
}

static void StripEndNewlineFromString( char *str )
{
	int s = strlen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == '\n' || str[s] == '\r' )
			str[s] = 0;
	}
}

static void StripEndNewlineFromString( wchar_t *str )
{
	int s = wcslen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == L'\n' || str[s] == L'\r' )
			str[s] = 0;
	}
}

DECLARE_HUDELEMENT( CHudChat );

DECLARE_HUD_MESSAGE( CHudChat, SayText );
DECLARE_HUD_MESSAGE( CHudChat, TextMsg );


//=====================
//CHudChatLine
//=====================

void CHudChatLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetProportional( true );

	m_hFont = pScheme->GetFont( "ChatFont", true );
	SetBorder( NULL );
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );

	SetFont( m_hFont );

	// get the team colours from the scheme
	m_CWChatColor	= pScheme->GetColor( "Chat.CommonWealthColor", Color( 0, 255, 0, 255 ) );
	m_AXChatColor	= pScheme->GetColor( "Chat.AxisColor", Color( 255, 0, 0, 255 ) );
	m_SPChatColor	= pScheme->GetColor( "Chat.SpectatorWealthColor", Color( 255, 255, 0, 255 ) );
	m_ChatColor		= pScheme->GetColor( "Chat.TextColor", GetFgColor() );
}

void CHudChatLine::PerformFadeout( void )
{
	// Flash + Extra bright when new
	float curtime = gpGlobals->curtime;

	int lr = m_clrText[0];
	int lg = m_clrText[1];
	int lb = m_clrText[2];
	
	//CSPort chat only fades out, no blinking.
	if ( curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME )
	{
		float frac = ( m_flExpireTime - curtime ) / CHATLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp( alpha, 0, 255 );

		wchar_t wbuf[4096];
		
		GetText(0, wbuf, sizeof(wbuf));

		SetText( "" );
			
		if ( m_iNameLength > 0 )
		{
			wchar_t wText[4096];
			// draw the first x characters in the player color
			wcsncpy( wText, wbuf, min( m_iNameLength + 1, MAX_PLAYER_NAME_LENGTH+32) );
			wText[ min( m_iNameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;

			m_clrNameColor[3] = alpha;
			InsertColorChange( m_clrNameColor );
			InsertString( wText );

			wcsncpy( wText, wbuf + ( m_iNameLength ), wcslen( wbuf + m_iNameLength ) );
			wText[ wcslen( wbuf + m_iNameLength ) ] = '\0';
			
			int c[3];
			m_ChatColor.GetColor( c[0], c[1], c[2], c[3] );
			
			InsertColorChange( Color( c[0], c[1], c[2], alpha ) );
			InsertString( wText );
			InvalidateLayout( true );
		}
		else
		{
			InsertColorChange( Color( lr, lg, lb, alpha ) );
			InsertString( wbuf );
		}
	}
	
	OnThink();
}



//=====================
//CHudChatInputLine
//=====================

void CHudChatInputLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui::HFont hFont = pScheme->GetFont( "ChatFontTyping", true );

	m_pPrompt->SetFont( hFont );
	m_pInput->SetFont( hFont );

	m_pInput->SetFgColor( pScheme->GetColor( "Chat.TypingText", Color( 0, 0, 0, 255 ) ) );
	m_pInput->SetBgColor( Color( 0, 0, 0, 150 ) );
}



//=====================
//CHudChat
//=====================

CHudChat::CHudChat( const char *pElementName ) : BaseClass( pElementName )
{
	// Set the hide flag bits
	SetHiddenBits( HIDEHUD_CHAT | HIDEHUD_MISCSTATUS /*| HIDEHUD_PLAYERDEAD */ );
	SetProportional( true );
	m_flBackgroundExpire = gpGlobals->curtime + 2.0f;
}

void CHudChat::CreateChatInputLine( void )
{
	m_pChatInput = new CHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );
}

void CHudChat::CreateChatLines( void )
{
	for ( int i = 0; i < CHAT_INTERFACE_LINES; i++ )
	{
		char sz[ 32 ];
		Q_snprintf( sz, sizeof( sz ), "ChatLine%02i", i );
		m_ChatLines[ i ] = new CHudChatLine( this, sz );
		m_ChatLines[ i ]->SetVisible( false );		
	}
}

void CHudChat::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );

	//SetPaintBackgroundEnabled( true );
	//SetPaintBorderEnabled( true );

	// get the team colours from the scheme
	m_CWChatColor	= pScheme->GetColor( "Chat.CommonWealthColor", Color( 0, 255, 0, 255 ) );
	m_AXChatColor	= pScheme->GetColor( "Chat.AxisColor", Color( 255, 0, 0, 255 ) );
	m_SPChatColor	= pScheme->GetColor( "Chat.SpectatorWealthColor", Color( 255, 255, 0, 255 ) );
	m_ChatColor		= pScheme->GetColor( "Chat.TextColor", GetFgColor() );

	GetPos( m_origxpos, m_origypos );
}	


void CHudChat::Init( void )
{
	BaseClass::Init();

//	HOOK_HUD_MESSAGE( CHudChat, RadioText );
	HOOK_HUD_MESSAGE( CHudChat, SayText );
	HOOK_HUD_MESSAGE( CHudChat, TextMsg );

	gameeventmanager->AddListener( this, "player_voice_command", false );

	bShouldDraw = false;
}

void CHudChat::FireGameEvent(IGameEvent *event)
{
	const char *type = event->GetName();

	if ( Q_strcmp( type, "player_voice_command" ) == 0 )
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

		if( player->GetTeamNumber() == event->GetInt( "teamid" ) ) // same team
		{
			char buffer[ 255 ];
			char ansi_local[512];

			Q_snprintf( buffer, 255, "#HaJ_Voice_%s", event->GetString( "vcommand" ) );

			C_HajPlayer *pPlayer = ToHajPlayer( C_BaseEntity::Instance( event->GetInt( "entindex" ) ) );
			C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();

			if( pLocalPlayer && pPlayer && pLocalPlayer != pPlayer ) // CHECK DISTANCE
			{
				if( pPlayer->IsDormant() )
					return;

				float flDistance = ( pPlayer->GetAbsOrigin() - pLocalPlayer->GetAbsOrigin() ).Length2D();

				if( flDistance > 600 )
					return;
			}

			if( strlen( buffer ) > 1 )
			{
				const wchar_t* localized = vgui::localize()->Find( buffer );
				vgui::localize()->ConvertUnicodeToANSI( localized, ansi_local, sizeof( ansi_local ) );

				ChatPrintf( event->GetInt( "entindex" ), "%s: (VOICE) %s", g_PR->GetPlayerName( event->GetInt( "entindex" ) ), ansi_local );
			}
		}
	}

	BaseClass::FireGameEvent(event);
}

//-----------------------------------------------------------------------------
// Purpose: Overrides base reset to not cancel chat at round restart
//-----------------------------------------------------------------------------
void CHudChat::Reset( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
void CHudChat::MsgFunc_SayText( bf_read &msg )
{
	char szString[256];

	int client = msg.ReadByte();
	msg.ReadString( szString, sizeof(szString) );
	bool bWantsToChat = msg.ReadByte();
	
	// flash speaking player dot
//	if ( client > 0 )
//		Radar_FlashPlayer( client );

	if ( bWantsToChat )
	{
		// print raw chat text
		 ChatPrintf( client, "%s", szString );
	}
	else
	{
		// try to lookup translated string
		 Printf( "%s", hudtextmessage->LookupString( szString ) );
	}

	Msg( "%s", szString );
}


// Message handler for text messages
// displays a string, looking them up from the titles.txt file, which can be localised
// parameters:
//   byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
//   string: message
// optional parameters:
//   string: message parameter 1
//   string: message parameter 2
//   string: message parameter 3
//   string: message parameter 4
// any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
// the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
void CHudChat::MsgFunc_TextMsg( bf_read &msg )
{
	char szString[2048];
	int msg_dest = msg.ReadByte();
	
	wchar_t szBuf[5][128];
	wchar_t outputBuf[256];

	for ( int i=0; i<5; ++i )
	{
		msg.ReadString( szString, sizeof(szString) );
		char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
		const wchar_t *pBuf = vgui::localize()->Find( tmpStr );
		if ( pBuf )
		{
			// Copy pBuf into szBuf[i].
			int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
			wcsncpy( szBuf[i], pBuf, nMaxChars );
			szBuf[i][nMaxChars-1] = 0;
		}
		else
		{
			if ( i )
			{
				StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
			}
			vgui::localize()->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
		}
	}

	if ( !cl_showtextmsg.GetInt() )
		return;

	int len;
	switch ( msg_dest )
	{
	case HUD_PRINTCENTER:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
		break;

	case HUD_PRINTNOTIFY:
		szString[0] = 1;  // mark this message to go into the notify buffer
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString+1, sizeof(szString)-1 );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;

	case HUD_PRINTTALK:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Printf( "%s", ConvertCRtoNL( szString ) );
		break;

	case HUD_PRINTCONSOLE:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;
	}
}

Color CHudChat::GetPlayerColor( int clientIndex )
{
	if ( clientIndex == 0 ) // console msg
	{
		return m_SPChatColor;
	}
	else if( g_PR )
	{
		switch ( g_PR->GetTeam( clientIndex ) )
		{
			// *HAJ 020* - Jed
			// HAJ Names
			case TEAM_AXIS		: return m_AXChatColor;
			case TEAM_CWEALTH	: return m_CWChatColor;
			default				: return m_SPChatColor;
		}
	}

	return m_SPChatColor;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CHudChat::ChatPrintf( int iPlayerIndex, const char *fmt, ... )
{
	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof( msg), fmt, marker);
	va_end(marker);

	// Strip any trailing '\n'
	if ( strlen( msg ) > 0 && msg[ strlen( msg )-1 ] == '\n' )
	{
		msg[ strlen( msg ) - 1 ] = 0;
	}

	// Strip leading \n characters ( or notify/color signifiers )
	char *pmsg = msg;
	while ( *pmsg && ( *pmsg == '\n' || *pmsg == 1 || *pmsg == 2 ) )
	{
		pmsg++;
	}
	
	if ( !*pmsg )
		return;

	CHudChatLine *line = (CHudChatLine *)FindUnusedChatLine();
	if ( !line )
	{
		ExpireOldest();
		line = (CHudChatLine *)FindUnusedChatLine();
	}

	if ( !line )
	{
		return;
	}

	line->SetText( "" );
	
	int iNameLength = 0;
	
	player_info_t sPlayerInfo;
	if ( iPlayerIndex == 0 )
	{
		Q_memset( &sPlayerInfo, 0, sizeof(player_info_t) );
		Q_strncpy( sPlayerInfo.name, "Console", sizeof(sPlayerInfo.name)  );	
	}
	else
	{
		engine->GetPlayerInfo( iPlayerIndex, &sPlayerInfo );
	}	

	const char *pName = sPlayerInfo.name;

	if ( pName )
	{
		const char *nameInString = strstr( pmsg, pName );

		if ( nameInString )
		{
			iNameLength = strlen( pName ) + (nameInString - pmsg);
		}
	}
	else
		//line->InsertColorChange( Color( g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2], 255 ) );
		line->InsertColorChange( m_SPChatColor );

	char *buf = static_cast<char *>( _alloca( strlen( pmsg ) + 1  ) );
	wchar_t *wbuf = static_cast<wchar_t *>( _alloca( (strlen( pmsg ) + 1 ) * sizeof(wchar_t) ) );
	if ( buf )
	{
		//float *flColor = GetClientColor( iPlayerIndex );
		Color playerColor = GetPlayerColor( iPlayerIndex );

		line->SetExpireTime();
	
		// draw the first x characters in the player color
		Q_strncpy( buf, pmsg, min( iNameLength + 1, MAX_PLAYER_NAME_LENGTH+32) );
		buf[ min( iNameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;
		
		//line->InsertColorChange( Color( flColor[0], flColor[1], flColor[2], 255 ) );
		line->InsertColorChange( playerColor );
		
		line->InsertString( buf );
		Q_strncpy( buf, pmsg + iNameLength, strlen( pmsg ));
		buf[ strlen( pmsg + iNameLength ) ] = '\0';
		
		//line->InsertColorChange( Color( g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2], 255 ) );
		line->InsertColorChange( m_ChatColor );

		vgui::localize()->ConvertANSIToUnicode( buf, wbuf, strlen(pmsg)*sizeof(wchar_t));
		line->InsertString( wbuf );
		line->SetVisible( true );
		line->SetNameLength( iNameLength );
		
		//line->SetNameColor( Color( flColor[0], flColor[1], flColor[2], 255 ) );
		line->SetNameColor( playerColor );
	}

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, "HudChat.Message" );
}

int CHudChat::GetChatInputOffset( void )
{
	if ( m_pChatInput->IsVisible() )
	{
		return m_iFontHeight;
	}
	else
		return 0;
}

void CHudChat::Paint( void )
{
#ifndef _XBOX
	
	// should we bother drawing?
	SetVisible ( bShouldDraw );
	if ( !bShouldDraw ) return;

	// calculate the alpha for the hud background
	float frac = ( m_flBackgroundExpire - gpGlobals->curtime ) / CHATLINE_FADE_TIME;
	int alpha = frac * 255;
	alpha = clamp( alpha, 0, 255 );

	// if its zero, bail here
	if ( alpha <= 0 )
	{
		bShouldDraw = true;
		return;
	}

	SetPos( m_xpos, m_ypos );

	if ( m_pBackground != NULL  )
		m_pBackground->DrawSelf( 0, 0, GetWide(), GetTall(), Color(255, 255, 255, alpha) );
#endif
	
	BaseClass::Paint();
}

void CHudChat::VidInit( void )
{
	m_pBackground =	gHUD.GetIcon( m_szBgTexture );
}

bool CHudChat::HasVisibleLines( void )
{
	int vl = 0;
	// see if any of our lines are still showing
	for ( int i = 0; i < CHAT_INTERFACE_LINES; i++ )
	{
		CBaseHudChatLine *line = m_ChatLines[ i ];
		if ( !line )
			continue;

		if ( !line->IsVisible() )
			continue;
		
		vl++;
	}
			
	// we've no lines showing so dont bother drawing
	if ( vl <= 0) 
		return false;
	else
		return true;
}	

void CHudChat::OnThink( void )
{
	BaseClass::OnThink();

	// set our flag to false to begin with
	bShouldDraw = false;

	// bail out tests
	if ( !CHudElement::ShouldDraw() )
		return;
	
	if ( engine->IsDrawingLoadingImage() )
		return;

	if ( !engine->IsInGame() )
		return;

	float curtime = gpGlobals->curtime;		// time now

	// move the chat panel if the spectator panel is visible
	CHajGameRules *pRules = HajGameRules();

	IViewPortPanel *pSpecGUI = gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
	if( (pSpecGUI && pSpecGUI->IsVisible()) || (pRules && pRules->IsGameOver()) )
	{
		m_xpos = 20;
		m_ypos = (m_origypos - GetTall()) - ((ScreenHeight()/480) * 15 );
	}
	else
	{
		m_xpos = m_origxpos;
		m_ypos = m_origypos;
	}
	// If the input box is showing we don't want to expire this panel
	// so we'll set the expire time as two seconds from now.
	if ( m_pChatInput->IsVisible() || HasVisibleLines() )
	{
		m_flBackgroundExpire = curtime + 1.0f;
	}
	
	// ok we past all tests so set flag to true
	bShouldDraw = true;
}