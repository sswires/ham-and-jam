//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "clientmode_hl2mpnormal.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"

#include "haj_capturepoint_c.h"

#include "util_shared.h"
#include "haj_gamerules.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_deathnotice_time( "hud_deathnotice_time", "6", 0 );
static ConVar hud_deathnotice_iconscale( "hud_deathnotice_iconscale", "0.75" );

// Player entries in a death notice
struct DeathNoticePlayer
{
	char		szName[150];
	int			iEntIndex;
};

// Contents of each entry in our list of death notices
struct DeathNoticeItem 
{
	DeathNoticePlayer	Killer;
	DeathNoticePlayer   Victim;
	CHudTexture *iconDeath;
	int			iSuicide;
	float		flDisplayTime;
	bool		bHeadshot;

	// capture
	bool		bIsCapture;
	int			iCaptureTeam;
	int			iOriginalTeam;

	DeathNoticeItem()
	{
		flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();
	}
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudDeathNotice : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDeathNotice, vgui::Panel );
public:
	CHudDeathNotice( const char *pElementName );
	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetColorForNoticePlayer( int iTeamNumber );
	void RetireExpiredDeathNotices( void );
	
	virtual void FireGameEvent( IGameEvent * event );
	void HandlePointUnderAttack( IGameEvent * event );
	void HandleZoneCapture( IGameEvent * event );
	void HandlePointDefend( IGameEvent * event );
	void HandlePlantedTNT( IGameEvent* event );

	CHudTexture* GetHUDTextureForPoint( int iEntID, int iTeam );
	int FindOrCreateTexID( char* texname );

private:

	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "15", "proportional_float" );

	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );

	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudDeathMsg" );

	// Texture for skull symbol
	CHudTexture		*m_iconD_skull;  
	CHudTexture		*m_iconD_headshot;  

	CUtlVector<DeathNoticeItem> m_DeathNotices;
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudDeathNotice );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudDeathNotice::CHudDeathNotice( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudDeathNotice" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iconD_headshot = NULL;
	m_iconD_skull = NULL;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Init( void )
{
	gameeventmanager->AddListener(this, "player_death", false );
	gameeventmanager->AddListener(this, "zone_captured", false );
	gameeventmanager->AddListener( this, "zone_pointunderattack", false );
	gameeventmanager->AddListener( this, "zone_defended", false );
	gameeventmanager->AddListener( this, "zone_plantedtnt", false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::VidInit( void )
{
	m_iconD_skull = gHUD.GetIcon( "d_skull" );
	m_DeathNotices.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Draw if we've got at least one death notice in the queue
//-----------------------------------------------------------------------------
bool CHudDeathNotice::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && ( m_DeathNotices.Count() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::SetColorForNoticePlayer( int iTeamNumber )
{
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( iTeamNumber ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Paint()
{
	if ( !m_iconD_skull )
		return;

	int yStart = GetClientModeHL2MPNormal()->GetDeathMessageStartHeight();

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( 0 ) );


	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		CHudTexture *icon = m_DeathNotices[i].iconDeath;
		if ( !icon )
			continue;
		wchar_t victim[ 256 ];
		wchar_t killer[ 256 ];

		// Get the team numbers for the players involved
		int iKillerTeam = 0;
		int iVictimTeam = 0;

		if( m_DeathNotices[i].bIsCapture == true )
		{
			iKillerTeam = m_DeathNotices[i].iCaptureTeam;
			iVictimTeam = m_DeathNotices[i].iOriginalTeam;
		}
		else
		{
			if( g_PR )
			{
				if( m_DeathNotices[i].Killer.iEntIndex <= MAX_PLAYERS )
				{
					iKillerTeam = g_PR->GetTeam( m_DeathNotices[i].Killer.iEntIndex );
				}
				else
				{
					iKillerTeam = TEAM_UNASSIGNED;
				}
				
				if( m_DeathNotices[i].Victim.iEntIndex <= MAX_PLAYERS )
				{
					iVictimTeam = g_PR->GetTeam( m_DeathNotices[i].Victim.iEntIndex );
				}
				else
				{
					iVictimTeam = TEAM_UNASSIGNED;
				}
			}
		}

		vgui::localize()->ConvertANSIToUnicode( m_DeathNotices[i].Victim.szName, victim, sizeof( victim ) );
		vgui::localize()->ConvertANSIToUnicode( m_DeathNotices[i].Killer.szName, killer, sizeof( killer ) );

		// Get the local position for this notice
		int len = UTIL_ComputeStringWidth( m_hTextFont, victim );
		int y = yStart + (m_flLineHeight * i);

		int iconWide;
		int iconTall;

		if( icon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( icon->hFont, icon->cCharacterInFont );
			iconTall = surface()->GetFontTall( icon->hFont );
		}
		else
		{
			float dimensionScale = ( ( m_flLineHeight - YRES(2) ) / icon->Height() ) * hud_deathnotice_iconscale.GetFloat();
			iconTall = ( icon->Height() * dimensionScale ) ;
			iconWide = ( icon->Width() * dimensionScale );
		}

		int x;
		if ( m_bRightJustify )
		{
			/*if( m_DeathNotices[i].bIsCapture == true)
			{
				x =	GetWide() - len - 52;
			}
			else*/
			{
				x =	GetWide() - len - iconWide - XRES( 14 );
			}
		}
		else
		{
			x = 0;
		}
		
		// Only draw killers name if it wasn't a suicide
		if ( !m_DeathNotices[i].iSuicide )
		{
			if ( m_bRightJustify )
			{
				x -= UTIL_ComputeStringWidth( m_hTextFont, killer );
			}

			SetColorForNoticePlayer( iKillerTeam );

			// Draw killer's name
			surface()->DrawSetTextPos( x, y );
			surface()->DrawSetTextFont( m_hTextFont );
			surface()->DrawUnicodeString( killer );
			surface()->DrawGetTextPos( x, y );
		}

		Color iconColor( 200, 200, 200, 255 );

		// Draw death weapon
		//If we're using a font char, this will ignore iconTall and iconWide

		/*if( m_DeathNotices[i].bIsCapture == true)
		{
			icon->DrawSelf( x+10, y, 32, 32, Color( 255, 255, 255, 255 ) );
			x += 52;		
		}
		else*/
		{
			x += XRES( 7 );
			icon->DrawSelf( x, y, iconWide, iconTall, iconColor );
			x += iconWide + XRES( 7 );		
		}

		SetColorForNoticePlayer( iVictimTeam );

		// Draw victims name
		surface()->DrawSetTextPos( x, y );
		surface()->DrawSetTextFont( m_hTextFont );	//reset the font, draw icon can change it
		surface()->DrawUnicodeString( victim );
	}

	// Now retire any death notices that have expired
	RetireExpiredDeathNotices();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CHudDeathNotice::RetireExpiredDeathNotices( void )
{
	// Loop backwards because we might remove one
	int iSize = m_DeathNotices.Size();
	for ( int i = iSize-1; i >= 0; i-- )
	{
		if ( m_DeathNotices[i].flDisplayTime < gpGlobals->curtime )
		{
			m_DeathNotices.Remove(i);
		}
	}
}


void CHudDeathNotice::HandlePointUnderAttack( IGameEvent * event )
{
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( !pPlayer ) return;

	// only show if local team owns the point
	if( event->GetInt( "ownerteam") == pPlayer->GetTeamNumber() )
	{
		DeathNoticeItem deathUnderAttack;

		deathUnderAttack.bIsCapture = true;
		deathUnderAttack.iSuicide = false;
		deathUnderAttack.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

		Q_strncpy( deathUnderAttack.Killer.szName, event->GetString( "zone" ), 150 );
		Q_strncpy( deathUnderAttack.Victim.szName, "is under attack!", 150 );
		deathUnderAttack.Killer.iEntIndex = -1;
		deathUnderAttack.Victim.iEntIndex = -1;

		deathUnderAttack.iCaptureTeam = event->GetInt( "ownerteam" );
		deathUnderAttack.iOriginalTeam = event->GetInt( "teamid" );

		deathUnderAttack.iconDeath = GetHUDTextureForPoint( event->GetInt( "entityid" ), event->GetInt( "ownerteam" ) );

		if( !deathUnderAttack.iconDeath )
			deathUnderAttack.iconDeath = gHUD.GetIcon( "hud_flag_blank" );

		m_DeathNotices.AddToTail( deathUnderAttack );
	}
}

void CHudDeathNotice::HandleZoneCapture( IGameEvent * event )
{
	DevMsg( "Got point capture\n" );

	DeathNoticeItem deathCapture;
	deathCapture.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

	// assemble a list of the capturing players
	int nPlayers = event->GetInt("players");
	char szNames[150];
	memset( szNames, 0x0, sizeof( szNames ) );

	C_BasePlayer::GetLocalPlayer()->EmitSound( "HaJGame.PointCaptured" );

	if( nPlayers < 1 )
	{
		DevMsg( 1, "nPlayers < 1\n" );
		Q_strcat(szNames, "Unknown", 150);
	}
	else
	{
		for(int iPlayer = 0 ; iPlayer < nPlayers ; ++iPlayer)
		{
			char useridstring[10];
			sprintf( useridstring, "userid%d", iPlayer );

			int user = engine->GetPlayerForUserID( event->GetInt(useridstring) );

			Q_strcat( szNames, g_PR->GetPlayerName( user ), sizeof( szNames ) );

			if( g_PR->IsLocalPlayer( user ) )
			{
				deathCapture.flDisplayTime = gpGlobals->curtime + ( hud_deathnotice_time.GetFloat() * 1.5 );
				DevMsg( "Local player in cap area\n" );
			}

			// check if we need to seperate player names
			if(nPlayers > 1 && iPlayer < nPlayers - 1)
			{
				Q_strcat(szNames, " / ", sizeof( szNames ));
			}
		}
	}
	DevMsg( 1, "Constructed cap attackers string: %s\n", szNames );

	wchar_t wszCapture[150], wszCapPoint[80];
	vgui::localize()->ConvertANSIToUnicode( event->GetString( "zone"), wszCapPoint, sizeof(wszCapPoint) );
	vgui::localize()->ConstructString( wszCapture, sizeof(wszCapture), vgui::localize()->Find( "#HaJ_CapturedArea" ), 1, wszCapPoint );

	deathCapture.Victim.iEntIndex = -1;
	Q_snprintf( deathCapture.Victim.szName, sizeof(deathCapture.Victim.szName), "%S", wszCapture );
	Q_strncpy( deathCapture.Killer.szName, szNames, 150 );

	deathCapture.Killer.iEntIndex = engine->GetPlayerForUserID( event->GetInt("userid0") );;
	deathCapture.bIsCapture = true;
	deathCapture.iSuicide = false;
	deathCapture.iCaptureTeam = event->GetInt( "teamid" );

	int iEntIndex = event->GetInt( "entityid" );
	//C_BaseEntity *pCap = C_BaseEntity::Instance( iEntIndex );
	//C_CapturePoint *pCapturePoint = dynamic_cast< C_CapturePoint* >( pCap );

	int iTeam = event->GetInt( "teamid" );
	int iOriginalTeam = event->GetInt( "origteam" );

	deathCapture.iOriginalTeam = iOriginalTeam;
	deathCapture.iconDeath = GetHUDTextureForPoint( iEntIndex, iTeam );

	if( !deathCapture.iconDeath )
		deathCapture.iconDeath = gHUD.GetIcon( "hud_flag_blank" );

	m_DeathNotices.AddToTail( deathCapture );
}

void CHudDeathNotice::HandlePointDefend( IGameEvent * event )
{
	DevMsg( "Got point defense\n" );

	DeathNoticeItem deathCapture;
	deathCapture.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

	int user = engine->GetPlayerForUserID( event->GetInt( "defender" ) );

	Q_strncpy( deathCapture.Killer.szName, g_PR->GetPlayerName( user ), 150 );
	Q_strncpy( deathCapture.Victim.szName, event->GetString( "zone" ), 150 );

	deathCapture.Killer.iEntIndex = -1;
	deathCapture.Victim.iEntIndex = -1;

	deathCapture.bIsCapture = true;
	deathCapture.iSuicide = false;

	deathCapture.iCaptureTeam = g_PR->GetTeam( user );
	deathCapture.iOriginalTeam = event->GetInt( "teamid" );

	deathCapture.iconDeath = gHUD.GetIcon( "obj_defend" );


	m_DeathNotices.AddToTail( deathCapture );
}

void CHudDeathNotice::HandlePlantedTNT( IGameEvent* event )
{
	DevMsg( "Got bomb planted\n" );

	DeathNoticeItem deathCapture;
	deathCapture.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

	int user = engine->GetPlayerForUserID( event->GetInt( "planter" ) );

	wchar_t vicbuffer[150], wszCapName[80];
	vgui::localize()->ConvertANSIToUnicode( event->GetString("zone"), wszCapName, sizeof( wszCapName ) );
	vgui::localize()->ConstructString( vicbuffer, sizeof(vicbuffer), vgui::localize()->Find( "#HaJ_ExplosivesPlanted" ), 1, wszCapName );
	
	Q_snprintf( deathCapture.Victim.szName, sizeof(deathCapture.Victim.szName), "%S", vicbuffer);
	Q_strncpy( deathCapture.Killer.szName, g_PR->GetPlayerName( user ), 150 );

	deathCapture.Killer.iEntIndex = -1;
	deathCapture.Victim.iEntIndex = -1;

	deathCapture.bIsCapture = true;
	deathCapture.iSuicide = false;

	deathCapture.iCaptureTeam = g_PR->GetTeam( user );
	deathCapture.iOriginalTeam = event->GetInt( "teamid" );

	deathCapture.iconDeath = GetHUDTextureForPoint( event->GetInt( "entityid" ), event->GetInt( "teamid" ) );

	m_DeathNotices.AddToTail( deathCapture );
}

//-----------------------------------------------------------------------------
// Purpose: Server's told us that someone's died
//-----------------------------------------------------------------------------
void CHudDeathNotice::FireGameEvent( IGameEvent * event )
{
	if (!g_PR)
		return;

	if ( hud_deathnotice_time.GetFloat() == 0 )
		return;

	DevMsg( "Got event %s\n", event->GetName() );

	if( !strcmp( "zone_pointunderattack", event->GetName() ) )
	{
		HandlePointUnderAttack( event );
		return;
	}
	else if( !strcmp( "zone_plantedtnt", event->GetName() ) )
	{
		HandlePlantedTNT( event );
		return;
	}
	else if( !strcmp( "zone_captured", event->GetName()))
	{
		HandleZoneCapture( event );
		return;
	}
	else if( !strcmp( "zone_defended", event->GetName() ) )
	{
		HandlePointDefend( event );
		return;
	}

	// the event should be "player_death"
	int killer = engine->GetPlayerForUserID( event->GetInt("attacker") );
	int victim = engine->GetPlayerForUserID( event->GetInt("userid") );
	int damaget = event->GetInt( "damagetype" );

	const char *killedwith = event->GetString( "weapon" );

	char fullkilledwith[128];
	if ( killedwith && *killedwith )
	{
		Q_snprintf( fullkilledwith, sizeof(fullkilledwith), "death_%s", killedwith );
	}
	else
	{
		fullkilledwith[0] = 0;
	}

	// Do we have too many death messages in the queue?
	if ( m_DeathNotices.Count() > 0 &&
		m_DeathNotices.Count() >= (int)m_flMaxDeathNotices )
	{
		// Remove the oldest one in the queue, which will always be the first
		m_DeathNotices.Remove(0);
	}

	// Get the names of the players

	char killer_name[255];
	strcpy( killer_name, g_PR->GetPlayerName( killer ) );

	char victim_name[255];
	strcpy( victim_name, g_PR->GetPlayerName( victim ) );

	if ( killer == 0 || !killer_name || Q_strcmp( killer_name, "unconnected" ) == 0 )
	{
		if( killedwith )
		{
			char buffer[MAX_PLAYER_NAME_LENGTH];
			Q_snprintf( buffer, MAX_PLAYER_NAME_LENGTH, "#killmsg_%s", killedwith );
			
			const wchar_t* localized = vgui::localize()->Find( buffer );
			vgui::localize()->ConvertUnicodeToANSI( localized, killer_name, sizeof( killer_name));

			// no string found
			if ( !killer_name || Q_strcmp( killer_name, "unconnected" ) == 0 )
				Q_snprintf( killer_name, sizeof( killer_name ), "" );
		}
		else
		{
			Q_snprintf( killer_name, sizeof( killer_name ), "" );
		}
	}
	if ( !victim_name )
		Q_snprintf( victim_name, sizeof( victim_name ), "" );

	// Make a new death notice
	DeathNoticeItem deathMsg;

	if( killer )
	{
		deathMsg.Killer.iEntIndex = killer;
	}

	deathMsg.Victim.iEntIndex = victim;
	Q_strncpy( deathMsg.Killer.szName, killer_name, MAX_PLAYER_NAME_LENGTH );
	Q_strncpy( deathMsg.Victim.szName, victim_name, MAX_PLAYER_NAME_LENGTH );

	if( g_PR->IsLocalPlayer( killer ) || g_PR->IsLocalPlayer( victim ) )
		deathMsg.flDisplayTime = gpGlobals->curtime + ( hud_deathnotice_time.GetFloat() * 1.5 ); // display longer if it's our death or kill
	else
		deathMsg.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

	deathMsg.iSuicide = ( ( !killer && !killedwith ) || killer == victim );
	deathMsg.bIsCapture = false;

	// Try and find the death identifier in the icon list
	deathMsg.iconDeath = gHUD.GetIcon( fullkilledwith );

	if ( !deathMsg.iconDeath || ( deathMsg.iSuicide && (damaget & DMG_NEVERGIB) ) )
	{
		// Can't find it, so use the default skull & crossbones icon
		deathMsg.iconDeath = m_iconD_skull;
	}

	// Add it to our list of death notices
	m_DeathNotices.AddToTail( deathMsg );

	char sDeathMsg[512];

	// Record the death notice in the console
	if ( deathMsg.iSuicide )
	{
		if ( !strcmp( fullkilledwith, "d_worldspawn" ) )
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s died.\n", deathMsg.Victim.szName );
		}
		else	//d_world
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s suicided.\n", deathMsg.Victim.szName, fullkilledwith );
		}
	}
	else
	{
		Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s killed %s", deathMsg.Killer.szName, deathMsg.Victim.szName );

		if ( fullkilledwith && *fullkilledwith && (*fullkilledwith > 13 ) )
		{
			Q_strncat( sDeathMsg, VarArgs( " with %s (%s).\n", fullkilledwith+6, event->GetString( "inflictor" ) ), sizeof( sDeathMsg ), COPY_ALL_CHARACTERS );
		}
	}

	Msg( "%s", sDeathMsg );
}

CHudTexture* CHudDeathNotice::GetHUDTextureForPoint( int iEntID, int iTeam )
{
	C_BaseEntity *pCap = C_BaseEntity::Instance( iEntID );
	C_HajObjective *pCapturePoint = dynamic_cast< C_HajObjective* >( pCap );

	if( !pCapturePoint ) return NULL;

	char iconName[ 64 ] = "";
	int textureID = 0;

	if( iTeam == TEAM_CWEALTH )
		Q_strncpy( iconName, pCapturePoint->GetAlliesIcon(), sizeof( iconName ) );
	else if( iTeam == TEAM_AXIS )
		Q_strncpy( iconName, pCapturePoint->GetAxisIcon(), sizeof( iconName ) );
	else
		Q_strncpy( iconName, pCapturePoint->GetNeutralIcon(), sizeof( iconName ) );

	if( gHUD.GetIcon( iconName ) )
	{
		return gHUD.GetIcon( iconName );
	}
	else if( FindOrCreateTexID( iconName ) )
	{
		CHudTexture *pTemp = new CHudTexture();

		Q_strncpy( pTemp->szTextureFile, iconName, 64 );
		Q_strncpy( pTemp->szShortName, iconName, 64 );

		pTemp->textureId = textureID;
		pTemp->bRenderUsingFont = false;

		pTemp->rc.left = 0;
		pTemp->rc.top = 0;
		pTemp->rc.right	= 128;
		pTemp->rc.bottom = 128;

		return gHUD.AddSearchableHudIconToList( *pTemp );		
	}

	return gHUD.GetIcon( "hud_flag_blank" );

}

int CHudDeathNotice::FindOrCreateTexID( char* texname )
{
	// return the texture ID for the named texture or if we cant find it, create one.
	int id = vgui::surface()->DrawGetTextureId( texname );
	if ( id == -1 )
	{
		id = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( id, texname, false, true );
	}
	return id;
}