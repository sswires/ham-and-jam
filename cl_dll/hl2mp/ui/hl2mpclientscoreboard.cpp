//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hl2mpclientscoreboard.h"
#include "haj_player_c.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_hl2mp_player.h"
#include "hl2mp/backgroundpanel.h"
#include "hl2mp_gamerules.h"

#include <inetchannelinfo.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>
#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/ImagePanel.h>

#include "voice_status.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::CHL2MPClientScoreBoardDialog( IViewPort *pViewPort ) : CClientScoreBoardDialog( pViewPort )
{
	m_pPlayerListCommonwealth = new SectionedListPanel( this, "PlayerListCommonwealth" );
	m_pPlayerCountLabel_Commonwealth = new Label( this, "Commonwealth_PlayerCount", "" );
	m_pScoreLabel_Commonwealth = new Label( this, "Commonwealth_Score", "" );
	m_pPingLabel_Commonwealth = new Label( this, "Commonwealth_Latency", "" );

	m_pPlayerListAxis = new SectionedListPanel( this, "PlayerListAxis" );
	m_pPlayerCountLabel_Axis = new Label( this, "Axis_PlayerCount", "" );
	m_pScoreLabel_Axis = new Label( this, "Axis_Score", "" );
	m_pPingLabel_Axis = new Label( this, "Axis_Latency", "" );

	m_pVertLine = new ImagePanel( this, "VerticalLine" );

	m_pHostname = new Label( this, "ServerName", "" );
	m_pSpectators = new Label( this, "spectators", "" );

	m_pCwealthHeader = new CScoreboardTeamHeader( this, "Commonwealth_TeamHeader" );
	m_pAxisHeader = new CScoreboardTeamHeader( this, "Axis_TeamHeader" );

	m_iLocalPlayerItemID = -1;

	//ListenForGameEvent( "server_spawn" );
	gameeventmanager->AddListener( this, "server_spawn", false );

	SetDialogVariable( "server", "" );
	SetVisible( false );

	LoadControlSettings( "Resource/UI/scoreboard.res" );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::~CHL2MPClientScoreBoardDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint background for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = Color( 0, 0, 0, 200 );
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	m_pHostname->SetFgColor( Color( 255, 255, 255, 255 ) );

	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	// turn off the default player list since we have our own
	if ( m_pPlayerList )
	{
		m_pPlayerList->SetVisible( false );
	}

	m_pScoreHeader_Commonwealth = (Label*)FindChildByName( "Commonwealth_ScoreHeader" );
	m_pDeathsHeader_Commonwealth = (Label*)FindChildByName( "Commonwealth_DeathsHeader" );
	m_pPingHeader_Commonwealth = (Label*)FindChildByName( "Commonwealth_PingHeader" );

	if ( m_pPlayerCountLabel_Commonwealth && m_pScoreHeader_Commonwealth && m_pScoreLabel_Commonwealth && m_pDeathsHeader_Commonwealth && m_pPingHeader_Commonwealth && m_pPingLabel_Commonwealth )
	{
		m_pPlayerCountLabel_Commonwealth->SetFgColor( COLOR_GREEN );
		m_pScoreLabel_Commonwealth->SetFgColor( COLOR_GREEN );
		m_pPingLabel_Commonwealth->SetFgColor( COLOR_GREEN );
	}

	m_pScoreHeader_Axis = (Label*)FindChildByName( "Axis_ScoreHeader" );
	m_pDeathsHeader_Axis = (Label*)FindChildByName( "Axis_DeathsHeader" );
	m_pPingHeader_Axis = (Label*)FindChildByName( "Axis_PingHeader" );

	if ( m_pPlayerCountLabel_Axis && m_pScoreHeader_Axis && m_pScoreLabel_Axis && m_pDeathsHeader_Axis && m_pPingHeader_Axis && m_pPingLabel_Axis )
	{
		m_pPlayerCountLabel_Axis->SetFgColor( COLOR_RED );
		m_pScoreLabel_Axis->SetFgColor( COLOR_RED );
		m_pPingLabel_Axis->SetFgColor( COLOR_RED );
	}

	// Store the scoreboard width, for Update();
	m_iStoredScoreboardWidth = GetWide();

	Reset();
	SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Resets the scoreboard panel
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::Reset()
{
	InitPlayerList( m_pPlayerListCommonwealth, TEAM_CWEALTH );
	InitPlayerList( m_pPlayerListAxis, TEAM_AXIS );
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::HL2MPPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1 = list->GetItemData( itemID1 );
	KeyValues *it2 = list->GetItemData( itemID2 );
	Assert( it1 && it2 );

	// first compare the players ce
	int v1 = it1->GetInt( "ce_f" );
	int v2 = it2->GetInt( "ce_f" );
	if ( v1 > v2 )
		return true;
	else if ( v1 < v2 )
		return false;

	// second compare objectives
	v1 = it1->GetInt( "objectives" );
	v2 = it2->GetInt( "objectives" );
	if ( v1 > v2 )
		return true;
	else if ( v1 < v2 )
		return false;

	// third compare actual kills
	v1 = it1->GetInt( "frags" );
	v2 = it2->GetInt( "frags" );
	if ( v1 > v2 )
		return true;
	else if ( v1 < v2 )
		return false;

	// if score and deaths are the same, use player index to get deterministic sort
	int iPlayerIndex1 = it1->GetInt( "playerIndex" );
	int iPlayerIndex2 = it2->GetInt( "playerIndex" );
	return ( iPlayerIndex1 > iPlayerIndex2 );
}

//-----------------------------------------------------------------------------
// Purpose: Inits the player list in a list panel
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::InitPlayerList( SectionedListPanel *pPlayerList, int teamNumber )
{
	pPlayerList->SetProportional( true );
	pPlayerList->SetVerticalScrollbar( false );
	pPlayerList->RemoveAll();
	pPlayerList->RemoveAllSections();
	pPlayerList->AddSection( 0, "Players", HL2MPPlayerSortFunc );
	pPlayerList->SetSectionAlwaysVisible( 0, true );
	pPlayerList->SetSectionFgColor( 0, Color( 255, 255, 255, 255 ) );
	pPlayerList->SetBgColor( Color( 0, 0, 0, 0 ) );
	pPlayerList->SetBorder( NULL );

	// set the section to have the team color
	if ( teamNumber && GameResources() )
	{
		pPlayerList->SetSectionFgColor( 0, GameResources()->GetTeamColor( teamNumber ) );
	}

	pPlayerList->AddColumnToSection( 0, "name", "", 0, m_iNameWidth );
	pPlayerList->AddColumnToSection( 0, "class", "" , 0, m_iClassWidth );
	//pPlayerList->AddColumnToSection( 0, "frags", "", SectionedListPanel::COLUMN_RIGHT, m_iScoreWidth );
	//pPlayerList->AddColumnToSection( 0, "deaths", "", SectionedListPanel::COLUMN_RIGHT, m_iDeathWidth );

	pPlayerList->AddColumnToSection( 0, "objectives", "", SectionedListPanel::COLUMN_RIGHT, m_iObjectivesWidth );
	pPlayerList->AddColumnToSection( 0, "ce", "", SectionedListPanel::COLUMN_RIGHT, m_iCEWidth );
	pPlayerList->AddColumnToSection( 0, "ping", "", SectionedListPanel::COLUMN_RIGHT, m_iPingWidth );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::Update()
{

	UpdateItemVisibiity();
	UpdateTeamInfo();
	UpdatePlayerList();
	UpdateSpectatorList();
	MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: Updates information about teams
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdateTeamInfo()
{
	// update the team sections in the scoreboard
	int startTeam = TEAM_UNASSIGNED;

	if ( HL2MPRules()->IsTeamplay() )
		startTeam = TEAM_CWEALTH;

	for ( int teamIndex = startTeam; teamIndex <= TEAM_AXIS; teamIndex++ )
	{
		// Make sure spectator is always skipped here.
		if ( teamIndex == TEAM_SPECTATOR )
			continue;

		wchar_t *teamName = NULL;
		C_Team *team = GetGlobalTeam( teamIndex );
		if ( team )
		{
			// choose dialog variables to set depending on team
			const char *pDialogVarTeamScore = NULL;
			const char *pDialogVarTeamPlayerCount = NULL;
			const char *pDialogVarTeamPing = NULL;
			switch ( teamIndex ) 
			{
			case TEAM_CWEALTH:
				teamName = vgui::localize()->Find( "#Haj_Team_CWealth" );
				pDialogVarTeamScore = "c_teamscore";
				pDialogVarTeamPlayerCount = "c_teamplayercount";
				pDialogVarTeamPing = "c_teamping";
				break;
			case TEAM_AXIS:
				teamName = vgui::localize()->Find( "#Haj_Team_Axis" );
				pDialogVarTeamScore = "a_teamscore";
				pDialogVarTeamPlayerCount = "a_teamplayercount";
				pDialogVarTeamPing = "a_teamping";
				break;

			default:
				Assert( false );
				break;
			}

			// update # of players on each team
			wchar_t name[64];
			wchar_t string1[1024];
			wchar_t wNumPlayers[6];
			_snwprintf( wNumPlayers, ARRAYSIZE( wNumPlayers ), L"%i", team->Get_Number_Players() );
			if ( !teamName && team )
			{
				vgui::localize()->ConvertANSIToUnicode( team->Get_Name(), name, sizeof( name ) );
				teamName = name;
			}
			if ( team->Get_Number_Players() == 1 )
			{
				vgui::localize()->ConstructString( string1, sizeof(string1), vgui::localize()->Find( "#ScoreBoard_Player" ), 2, teamName, wNumPlayers );
			}
			else
			{
				vgui::localize()->ConstructString( string1, sizeof(string1), vgui::localize()->Find( "#ScoreBoard_Players" ), 2, teamName, wNumPlayers );
			}

			// set # of players for team in dialog
			SetDialogVariable( pDialogVarTeamPlayerCount, string1 );

			// set team score in dialog
			if ( teamIndex != TEAM_UNASSIGNED )	// Don't accumulate deathmatch scores.
				SetDialogVariable( pDialogVarTeamScore, team->Get_Score() );			

			int pingsum = 0;
			int numcounted = 0;
			for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
			{
				if( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == teamIndex )
				{
					int ping = g_PR->GetPing( playerIndex );

					if ( ping >= 1 )
					{
						pingsum += ping;
						numcounted++;
					}
				}
			}

			if ( numcounted > 0 )
			{
				int ping = (int)( (float)pingsum / (float)numcounted );
				SetDialogVariable( pDialogVarTeamPing, ping );		
			}
			else
			{
				SetDialogVariable( pDialogVarTeamPing, "" );	
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the player list
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdatePlayerList()
{
	m_pPlayerListCommonwealth->RemoveAll();
	m_pPlayerListAxis->RemoveAll();

	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pLocalPlayer )
		return;

	for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if( g_PR->IsConnected( playerIndex ) )
		{
			SectionedListPanel *pPlayerList = NULL;

			// Not teamplay, use the DM playerlist
			switch ( g_PR->GetTeam( playerIndex ) )
			{
				case TEAM_CWEALTH:
					pPlayerList = m_pPlayerListCommonwealth;
					break;
				case TEAM_AXIS:
					pPlayerList = m_pPlayerListAxis;
					break;
			}

			if ( pPlayerList == NULL )
			{
				continue;			
			}

			KeyValues *pKeyValues = new KeyValues( "data" );
			GetPlayerScoreInfo( playerIndex, pKeyValues );

			int itemID = pPlayerList->AddItem( 0, pKeyValues );
			Color clr = g_PR->GetTeamColor( g_PR->GetTeam( playerIndex ) );
			
			if( g_PR->IsLocalPlayer( playerIndex ) ) // highlighting
			{
				m_iLocalPlayerItemID = itemID;
			}

			pPlayerList->SetItemFgColor( itemID, clr );
			pKeyValues->deleteThis();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the spectator list
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdateSpectatorList()
{
	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pLocalPlayer )
		return;

	char szSpectatorList[512] = "" ;
	int nSpectators = 0;
	for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if ( ShouldShowAsSpectator( playerIndex ) )
		{
			if ( nSpectators > 0 )
			{
				Q_strncat( szSpectatorList, ", ", ARRAYSIZE( szSpectatorList ) );
			}

			Q_strncat( szSpectatorList, g_PR->GetPlayerName( playerIndex ), ARRAYSIZE( szSpectatorList ) );
			nSpectators++;
		}
	}

	wchar_t wzSpectators[512] = L"";
	if ( nSpectators > 0 )
	{
		const char *pchFormat = ( 1 == nSpectators ? "#ScoreBoard_Spectator" : "#ScoreBoard_Spectators" );

		wchar_t wzSpectatorCount[16];
		wchar_t wzSpectatorList[1024];
		_snwprintf( wzSpectatorCount, ARRAYSIZE( wzSpectatorCount ), L"%i", nSpectators );
		vgui::localize()->ConvertANSIToUnicode( szSpectatorList, wzSpectatorList, sizeof( wzSpectatorList ) );
		vgui::localize()->ConstructString( wzSpectators, sizeof(wzSpectators), vgui::localize()->Find( pchFormat), 2, wzSpectatorCount, wzSpectatorList );
	}

	m_pSpectators->SetText( wzSpectators );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the specified player index is a spectator
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::ShouldShowAsSpectator( int iPlayerIndex )
{
	// see if player is connected
	if ( g_PR->IsConnected( iPlayerIndex ) ) 
	{
		// spectators show in spectator list
		int iTeam = g_PR->GetTeam( iPlayerIndex );

		// In team play the DM playerlist is invisible, so show unassigned in the spectator list.
		if ( HL2MPRules()->IsTeamplay() && TEAM_UNASSIGNED == iTeam )
			return true;

		if ( TEAM_SPECTATOR == iTeam )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Event handler
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();

	if ( 0 == Q_strcmp( type, "server_spawn" ) )
	{		
		INetChannelInfo *pInfo = engine->GetNetChannelInfo();

		// set server name in scoreboard
		const char *hostname = event->GetString( "hostname" );
		wchar_t wzHostName[256];
		wchar_t wzServerLabel[256];
		wchar_t wzServerIP[64];

		_snwprintf( wzServerIP, sizeof( wzServerIP ), L"%S", pInfo->GetAddress() );
		vgui::localize()->ConvertANSIToUnicode( hostname, wzHostName, sizeof( wzHostName ) );
		vgui::localize()->ConstructString( wzServerLabel, sizeof(wzServerLabel), vgui::localize()->Find( "#Scoreboard_Server" ), 2, wzHostName, wzServerIP );
		
		SetDialogVariable( "server", wzServerLabel );
	}

	if( IsVisible() )
	{
		Update();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
{
	// Clean up the player name
	const char *oldName = g_PR->GetPlayerName( playerIndex );
	int bufsize = strlen( oldName ) * 2 + 1;
	char *newName = (char *)_alloca( bufsize );
	UTIL_MakeSafeName( oldName, newName, bufsize );
	kv->SetString( "name", newName );

	kv->SetInt( "playerIndex", playerIndex );
	kv->SetInt( "frags", g_PR->GetPlayerScore( playerIndex ) );
	kv->SetInt( "deaths", g_PR->GetDeaths( playerIndex ) );

	// Class
	if( !g_PR->IsLocalPlayer( playerIndex ) && !g_PR->IsAlive( playerIndex ) )
	{
		kv->SetString( "class", "#HaJ_PlayerDead" );
	}
	else if( g_PR->GetTeam( playerIndex ) == C_HL2MP_Player::GetLocalHL2MPPlayer()->GetTeamNumber() )
	{
		switch( g_PR->GetPlayerClass( playerIndex ) )
		{
			case CLASS_RIFLEMAN:
				kv->SetString( "class", "#HaJ_Rifleman" );
				break;

			case CLASS_ASSAULT:
				kv->SetString( "class", "#HaJ_Assault" );
				break;

			case CLASS_SUPPORT:
				kv->SetString( "class", "#HaJ_Support" );
				break;
		}
	}
	else
	{
		kv->SetString( "class", "" );
	}

	kv->SetInt( "objectives", g_PR->GetObjectiveScore( playerIndex ) ); // objectives capped
	
	// formatted combat effectiveness
	char formatted_ce[20];
	sprintf( formatted_ce, "%d%%", g_PR->GetPlayerScore( playerIndex ) + 100 );

	kv->SetString( "ce", formatted_ce );
	kv->SetInt( "ce_f", g_PR->GetPlayerScore( playerIndex ) ); // for comparisons

	//UpdatePlayerAvatar( playerIndex, kv );

	if ( g_PR->GetPing( playerIndex ) < 1 )
	{
		if ( g_PR->IsFakePlayer( playerIndex ) )
		{
			kv->SetString( "ping", "BOT" );
		}
		else
		{
			kv->SetString( "ping", "" );
		}
	}
	else
	{
		kv->SetInt( "ping", g_PR->GetPing( playerIndex ) );
	}

	return true;
}

void CHL2MPClientScoreBoardDialog::UpdateItemVisibiity()
{
	// Need to do this in Update, ensure the correct player lists/headers are visible.
	if ( HL2MPRules()->IsTeamplay() )
	{
		// CWEALTH
		m_pPlayerListCommonwealth->SetVisible( true );
		m_pPlayerCountLabel_Commonwealth->SetVisible( true );
		m_pScoreHeader_Commonwealth->SetVisible( true );
		m_pScoreLabel_Commonwealth->SetVisible( false );
		m_pDeathsHeader_Commonwealth->SetVisible( true );
		m_pPingHeader_Commonwealth->SetVisible( true );
		m_pPingLabel_Commonwealth->SetVisible( false );

		// AXIS
		m_pPlayerListAxis->SetVisible( true );
		m_pPlayerCountLabel_Axis->SetVisible( true );
		m_pScoreHeader_Axis->SetVisible( true );
		m_pScoreLabel_Axis->SetVisible( false );
		m_pDeathsHeader_Axis->SetVisible( true );
		m_pPingHeader_Axis->SetVisible( true );
		m_pPingLabel_Axis->SetVisible( false );

		// Vertical Line _ON_
		m_pVertLine->SetVisible( true );

		// Restore the size to the original incase we've switched from DM -> Teams and back.
		SetSize(m_iStoredScoreboardWidth, GetTall() );
	}

}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::SetLabelText(const char *textEntryName, wchar_t *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
		entry->SetFont( m_hLabelFont ); // *HAJ 020* - Jed - Set Font
	}
}


void CHL2MPClientScoreBoardDialog::Paint()
{
	// local player highlighting
	if( m_iLocalPlayerItemID > -1 )
	{
		CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

		if( pLocalPlayer )
		{
			SectionedListPanel *pPlayerList = NULL;

			// Not teamplay, use the DM playerlist
			switch ( pLocalPlayer->GetTeamNumber() )
			{
			case TEAM_CWEALTH:
				pPlayerList = m_pPlayerListCommonwealth;
				break;
			case TEAM_AXIS:
				pPlayerList = m_pPlayerListAxis;
				break;
			}

			if( pPlayerList )
			{
				int lpx, lpy, lpw, lph;
				pPlayerList->GetCellBounds( m_iLocalPlayerItemID, 0, lpx, lpy, lpw, lph );

				int plx, ply;
				pPlayerList->GetPos( plx, ply );

				surface()->DrawSetColor( m_HighlightColor );
				surface()->DrawFilledRect( plx, ply + lpy, lpx + pPlayerList->GetWide() + plx, ply + lpy + lph );
			}
		}
	}

	BaseClass::Paint();
}