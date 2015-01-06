// *HAJ 020* - Jed
// Re-worked class for team select menu. Based on Valve's original teammenu.cpp

#include "cbase.h"
#include "haj_teammenu.h"
#include "haj_teaminfo.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"
#include "haj_modelpanel.h"		// needed for model panels
#include "c_team.h"
#include "c_playerresource.h"
#include "BackgroundPanel.h"	// needed to draw the stock background
#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include "IGameUIFuncs.h" // for key bindings
#include <igameresources.h>

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

#include <cl_dll/iviewport.h>

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

void UpdateCursorState();

// helper function
const char *GetStringTeamColor( int i )
{
	switch( i )
	{
	case 0:
		return "team0";

	case 1:
		return "team1";

	case 2:
		return "team2";

	case 3:
		return "team3";

	case 4:
	default:
		return "team4";
	}
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTeamMenu::CTeamMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_TEAM )
{
	m_pViewPort = pViewPort;
	m_iJumpKey = -1; // this is looked up in Activate()
	m_iScoreBoardKey = -1; // this is looked up in Activate()
	m_iActiveTeamInfo = -1;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	
	SetProportional(true);
	SetVisible(false);

	// draw the stock background
	CreateBackground( this );
	m_backgroundLayoutFinished = false;

	// hide the system buttons
	SetTitleBarVisible( false );

	// info window about this map
	m_pMapSettings = NULL;
	m_pMapInfo = new RichText( this, "MapInfo" );
	m_pMapInfoHTML = new HTML( this, "MapInfoHTML");

	m_pTeamInfo = new Label( this, "TeamInfo", "" );

	m_pCwealthPropaganda = new LocalImageButton( this, "CwealthPropaganda", NULL, "jointeam 2" );
	m_pAxisPropaganda = new LocalImageButton( this, "AxisPropaganda", NULL, "jointeam 3" );
	m_pTeamFull = new LocalImageButton( this, "TeamFull", "team/buttons/full", NULL, false );

	m_pCwealthPropaganda->SetLocalAuxImage( "team/buttons/confirm" );
	m_pAxisPropaganda->SetLocalAuxImage( "team/buttons/confirm" );

	m_pCwealthPropaganda->SetMouseOutAction( "hideteaminfo" );
	m_pAxisPropaganda->SetMouseOutAction( "hideteaminfo" );

	m_pCwealthPropaganda->SetMouseOverAction( "showteaminfo 2" );
	m_pAxisPropaganda->SetMouseOverAction( "showteaminfo 3" );

	//m_pCwealthConfirm = new LocalImageButton( this, "CwealthConfirm", "team/buttons/confirm", "jointeam 2" );
	//m_pAxisConfirm = new LocalImageButton( this, "AxisConfirm", "team/buttons/confirm", "jointeam 3" );
	m_pSpectateButton = new LocalImageButton( this, "SpectateButton", "team/buttons/spectator", "spectate", false );
	m_pCancelButton = new LocalImageButton( this, "CancelButton", "team/buttons/cancel", "vguicancel", false );
	m_pAutoAssignButton = new LocalImageButton( this, "AutoAssignButton", "team/buttons/random", "jointeam 4", false );

	m_pTitle = new Label( this, "joinTeam", "#TM_Join_Team" );

	LoadControlSettings("Resource/UI/TeamMenu.res");
	InvalidateLayout();

	m_szMapName[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTeamMenu::~CTeamMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void CTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	//ApplyBackgroundSchemeSettings( this, pScheme );

	m_pMapInfo->SetFgColor( pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)) );
	m_pTitle->SetFont( pScheme->GetFont("HajPanelTitles", true ) );

	if ( *m_szMapName )
	{
		LoadMapPage( m_szMapName ); // reload the map description to pick up the color
	}

	m_pMapInfo->SetVerticalScrollbar(false);
	m_pMapInfo->SetBorder(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: makes the user choose the auto assign option
//-----------------------------------------------------------------------------
void CTeamMenu::AutoAssign()
{
	engine->ClientCmd("jointeam 4");
	Close();
}


//-----------------------------------------------------------------------------
// Purpose: shows the team menu
//-----------------------------------------------------------------------------
void CTeamMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	m_pViewPort->ShowBackGround( bShow );


	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );

		// get key bindings if shown
		if( m_iJumpKey < 0 ) // you need to lookup the jump key AFTER the engine has loaded
			m_iJumpKey = gameuifuncs->GetEngineKeyCodeForBind( "jump" );
	
		if ( m_iScoreBoardKey < 0 ) 
			m_iScoreBoardKey = gameuifuncs->GetEngineKeyCodeForBind( "showscores" );

		m_iActiveTeamInfo = -1;
		m_pMapInfo->SetVisible( false );
		m_pTeamInfo->SetVisible( false );

		CHajGameRules *pRules = HajGameRules();

		if( pRules )
		{
			pRules->ParseTeamInfo();

			m_pCwealthPropaganda->SetImage( pRules->GetTeamInfo(TEAM_CWEALTH)->GetPropagandaPoster() );
			m_pAxisPropaganda->SetImage( pRules->GetTeamInfo(TEAM_AXIS)->GetPropagandaPoster() );
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}

}

//-----------------------------------------------------------------------------
// Purpose: Called to set panel visible or not
//-----------------------------------------------------------------------------
void  CTeamMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );
}

//-----------------------------------------------------------------------------
// Purpose: updates the UI with a new map name and map html page, and sets up the team buttons
//-----------------------------------------------------------------------------
void CTeamMenu::Update()
{
	char mapname[MAX_MAP_NAME];

	Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );

	SetLabelText( "mapname", mapname );

	LoadMapPage( mapname );
}

extern ConVar haj_allowunbalancedteams;
void CTeamMenu::Think()
{
	// team full labels
	CHajGameRules *pRules = HajGameRules();
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( !haj_allowunbalancedteams.GetBool() && pRules && pPlayer )
	{
		C_Team *pCWealth = g_Teams[TEAM_CWEALTH];
		C_Team *pAxis = g_Teams[TEAM_AXIS];

		bool bOnCombatTeam = (bool)( pPlayer->GetTeamNumber() == TEAM_AXIS || pPlayer->GetTeamNumber() == TEAM_CWEALTH );
		m_bCWealthFull = (bool)( pCWealth->Get_Number_Players() > pAxis->Get_Number_Players() + ( bOnCombatTeam ? -1 : 0 ) );
		m_bAxisFull = (bool)( pAxis->Get_Number_Players() > pCWealth->Get_Number_Players() + ( bOnCombatTeam ? -1 : 0 ) );

		m_pAxisPropaganda->SetCommandEnabled( !m_bAxisFull );
		m_pCwealthPropaganda->SetCommandEnabled( !m_bCWealthFull );

		m_pAxisPropaganda->GetAuxImagePanel()->SetVisible( !m_bAxisFull );
		m_pCwealthPropaganda->GetAuxImagePanel()->SetVisible( !m_bCWealthFull );

		m_pAutoAssignButton->SetVisible( !bOnCombatTeam );

		vgui::LocalImageButton *pPoster = NULL;

		if( m_bAxisFull && pPlayer->GetTeamNumber() != TEAM_AXIS ) pPoster = m_pAxisPropaganda;
		else if( m_bCWealthFull && pPlayer->GetTeamNumber() != TEAM_CWEALTH ) pPoster = m_pCwealthPropaganda;

		if( pPoster )
		{
			int x, y, w, h;
			pPoster->GetBounds( x, y, w, h );

			m_pTeamFull->SetBounds( x + m_iTeamFullX, y + m_iTeamFullY, m_iTeamFullW, m_iTeamFullH );
			m_pTeamFull->SetVisible( true );
		}
		else
		{
			m_pTeamFull->SetVisible( false );
		}
	}
	else
	{
		m_pTeamFull->SetVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: The CS background is painted by image panels, so we should do nothing
//-----------------------------------------------------------------------------
void CTeamMenu::PaintBackground()
{
}

//-----------------------------------------------------------------------------
// Purpose: chooses and loads the text page to display that describes mapName map
//-----------------------------------------------------------------------------
void CTeamMenu::LoadMapPage( const char *mapName )
{
	// Save off the map name so we can re-load the page in ApplySchemeSettings().
	Q_strncpy( m_szMapName, mapName, strlen( mapName ) + 1 );
	
	char mapRES[ MAX_PATH ];

	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	Q_snprintf( mapRES, sizeof( mapRES ), "resource/maphtml/%s_%s.html", mapName, uilanguage );

	bool bFoundHTML = false;

	if ( !vgui::filesystem()->FileExists( mapRES ) )
	{
		// try english
		Q_snprintf( mapRES, sizeof( mapRES ), "resource/maphtml/%s_english.html", mapName );
	}
	else
	{
		bFoundHTML = true;
	}

	if( bFoundHTML || vgui::filesystem()->FileExists( mapRES ) )
	{
		// it's a local HTML file
		char localURL[ _MAX_PATH + 7 ];
		Q_strncpy( localURL, "file://", sizeof( localURL ) );

		char pPathData[ _MAX_PATH ];
		vgui::filesystem()->GetLocalPath( mapRES, pPathData, sizeof(pPathData) );
		Q_strncat( localURL, pPathData, sizeof( localURL ), COPY_ALL_CHARACTERS );

		// force steam to dump a local copy
		vgui::filesystem()->GetLocalCopy( pPathData );

		m_pMapInfo->SetVisible( false );

		m_pMapInfoHTML->SetVisible( true );
		m_pMapInfoHTML->OpenURL( localURL );

		InvalidateLayout();
		Repaint();		

		return;
	}
	else
	{
		//m_pMapInfo->SetVisible( true );
		m_pMapInfoHTML->SetVisible( false );
	}

	Q_snprintf( mapRES, sizeof( mapRES ), "maps/%s.txt", mapName);

	// if no map specific description exists, load default text
	if( !vgui::filesystem()->FileExists( mapRES ) )
	{
		if ( vgui::filesystem()->FileExists( "maps/default.txt" ) )
		{
			Q_snprintf ( mapRES, sizeof( mapRES ), "maps/default.txt");
		}
		else
		{
			m_pMapInfo->SetText( "" );
			return; 
		}
	}

	// *HAJ 020* - Ztormi
	// Since we have map script file named after the map we need to find the "description" keyvalue for the map text

	KeyValues *pkv = new KeyValues( "Mapinfo" );
	Assert( pkv );

	// Load the map script file
	if( pkv->LoadFromFile( vgui::filesystem(), mapRES, "MOD" ) )
	{
		DevMsg("Loading map script file from %s \n",mapRES );
	}
	else
	{
		Warning( "Unable to load map script file for '%s'\n", mapRES  );
		m_pMapInfo->SetText( "" );
		return;
	}

	//Read the keyvalues from file
	pkv->FindKey( "Mapinfo", false );
	
	/*if( pkv )
	{
			m_pMapInfo->SetText(pkv->GetString( "description", "" ));
	}*/

	m_pMapSettings = pkv->MakeCopy(); // there should be a copy constructor so
	pkv->deleteThis();
	//END Haj

	// This is the old maptext code - Ztormi

	/*
	FileHandle_t f = vgui::filesystem()->Open( mapRES, "r" );

	// read into a memory block
	int fileSize = vgui::filesystem()->Size(f);
	int dataSize = fileSize + sizeof( wchar_t );
	if ( dataSize % 2 )
		++dataSize;
	wchar_t *memBlock = (wchar_t *)malloc(dataSize);
	memset( memBlock, 0x0, dataSize);
	int bytesRead = vgui::filesystem()->Read(memBlock, fileSize, f);
	if ( bytesRead < fileSize )
	{
		// NULL-terminate based on the length read in, since Read() can transform \r\n to \n and
		// return fewer bytes than we were expecting.
		char *data = reinterpret_cast<char *>( memBlock );
		data[ bytesRead ] = 0;
		data[ bytesRead+1 ] = 0;
	}

	// null-terminate the stream (redundant, since we memset & then trimmed the transformed buffer already)
	memBlock[dataSize / sizeof(wchar_t) - 1] = 0x0000;

	// check the first character, make sure this a little-endian unicode file
	if (memBlock[0] != 0xFEFF)
	{
		// its a ascii char file
		m_pMapInfo->SetText( reinterpret_cast<char *>( memBlock ) );
	}
	else
	{
		m_pMapInfo->SetText( memBlock+1 );
	}

	// go back to the top of the text buffer

	vgui::filesystem()->Close( f );
	free(memBlock);*/

	//---END
	m_pMapInfo->GotoTextStart();

	InvalidateLayout();
	Repaint();

}

//-----------------------------------------------------------------------------
// Purpose: Trap button presses and issue commands.
// NOTE: Do we need to issue kill? Could we not do a graceful no suicide swap of
//       sides via the jointeam command?
//-----------------------------------------------------------------------------
void CTeamMenu::OnCommand( const char *command )
{

	// *HaJ 202* - SteveUK
	// Select a team and also make sure that the class menu shows up after choosing a team.
	if( Q_strstr( command, "showteaminfo" ) != false )
	{
		command += 13;
		int iTeam = atoi( command );

		if( m_pMapSettings )
		{
			switch( iTeam )
			{
			case TEAM_CWEALTH:
				m_pMapInfo->SetText( m_pMapSettings->GetWString( "cwealth_obj", m_pMapSettings->GetWString("description", L"") ) );
				break;

			case TEAM_AXIS:
				m_pMapInfo->SetText( m_pMapSettings->GetWString( "axis_obj", m_pMapSettings->GetWString("description", L"" ) ) );
				break;

			default:
				char ebuffer[64];
				Q_snprintf( ebuffer, sizeof( ebuffer), "%d - invalid team index", iTeam );

				m_pMapInfo->SetText( ebuffer );
			}

			m_pMapInfo->SetVisible( true );
		}

		int iTeamPlayers = g_Teams[iTeam]->Get_Number_Players();

		wchar_t wszTeamInfo[150], wszTeamPlayers[20], wszTeamName[120];
		_snwprintf( wszTeamPlayers, sizeof( wszTeamPlayers ), L"%d", iTeamPlayers );
		_snwprintf( wszTeamName, sizeof( wszTeamName ), L"%S", g_PR->GetTeamName(iTeam) );

		vgui::localize()->ConstructString( wszTeamInfo, sizeof( wszTeamInfo ), (iTeamPlayers != 1 ? vgui::localize()->Find( "#HaJ_TeamInfo_Multi" ) : vgui::localize()->Find( "#HaJ_TeamInfo_Single" )), 2, wszTeamName, wszTeamPlayers );
		
		m_pTeamInfo->SetText( wszTeamInfo );
		m_pTeamInfo->SetVisible( true );
		m_pTeamInfo->SizeToContents();

		m_iActiveTeamInfo = iTeam;

		return;
	}
	else if( Q_strstr( command, "hideteaminfo" ) != false)
	{
		m_pMapInfo->SetVisible( false );
		m_pTeamInfo->SetVisible( false );

		m_iActiveTeamInfo = -1;

		return;
	}
	else if ( Q_strstr( command, "jointeam" ) != false )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
		//gViewPortInterface->ShowPanel(PANEL_HAJCLASS, true); // we need to show the panel now so we can pick a class.
	}
	else
	{
		engine->ClientCmd( const_cast<char *>( command ) ); // just exec the command if it's not join team
	}
	
	Close();
	gViewPortInterface->ShowBackGround( false );
	gViewPortInterface->ShowPanel(PANEL_TEAM, false);
	
	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Scale / center the window
//-----------------------------------------------------------------------------
void CTeamMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	// stretch the window to fullscreen
	if ( !m_backgroundLayoutFinished )
		LayoutBackgroundPanel( this );
	m_backgroundLayoutFinished = true;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CTeamMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

void CTeamMenu::OnKeyCodePressed(KeyCode code)
{
	int lastPressedEngineKey = engine->GetLastPressedEngineKey();

	if( m_iJumpKey >= 0 && m_iJumpKey == lastPressedEngineKey )
	{
		AutoAssign();
	}
	else if ( m_iScoreBoardKey >= 0 && m_iScoreBoardKey == lastPressedEngineKey )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create any Model panels we use.
//-----------------------------------------------------------------------------
Panel *CTeamMenu::CreateControlByName(const char *controlName)
{
    if ( Q_stricmp( controlName, "ModelPanel" ) == 0 )
    {
        return new CModelPanel( NULL, controlName );
    }

    return BaseClass::CreateControlByName( controlName );
}