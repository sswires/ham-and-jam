#include "cbase.h"
#include "haj_classmenu.h"
#include "BackgroundPanel.h"
#include <cdll_client_int.h>

// player includes yay
#include "c_baseplayer.h"
#include "c_hl2mp_player.h"
#include "haj_gamerules.h"

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

#include "igameevents.h"

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

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHAJClassMenu::CHAJClassMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_HAJCLASS )
{
	m_bNeedsUpdate = false;
	m_pViewPort = pViewPort;
	m_iJumpKey = -1; // this is looked up in Activate()
	m_iScoreBoardKey = -1; // this is looked up in Activate()
	m_bInit = false;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	
	SetProportional(true);

	CreateBackground( this );
	m_backgroundLayoutFinished = false;

	// hide the system buttons
	SetTitleBarVisible( false );

	// info window about this map
	m_pMapInfo = new RichText( this, "MapInfo" );

	//int x,w,h;
	//GetBounds(x,x,w,h);
	//SetPos((ScreenWidth()-w)/2,(ScreenHeight()-h)/2);

	//LoadControlSettings("Resource/UI/cwealth_classmenu.res");

	InvalidateLayout();

	m_szMapName[0] = 0;
}

CHAJClassMenu::CHAJClassMenu(IViewPort *pViewPort, const char* panelName ) : Frame(NULL, panelName )
{
	m_bNeedsUpdate = true;
	m_pViewPort = pViewPort;
	m_iJumpKey = -1; // this is looked up in Activate()
	m_iScoreBoardKey = -1; // this is looked up in Activate()
	m_bInit = false;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetProportional(true);

	CreateBackground( this );
	m_backgroundLayoutFinished = false;

	// hide the system buttons
	SetTitleBarVisible( false );

	// info window about this map
	m_pMapInfo = new RichText( this, "MapInfo" );

	//int x,w,h;
	//GetBounds(x,x,w,h);
	//SetPos((ScreenWidth()-w)/2,(ScreenHeight()-h)/2);

	SetupTeam();
	InvalidateLayout();

	m_szMapName[0] = 0;

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHAJClassMenu::~CHAJClassMenu()
{
}

bool CHAJClassMenu::NeedsUpdate()
{
	if( m_bNeedsUpdate )
	{
		m_bNeedsUpdate = false;
		return true;
	}

	if( m_bInit )
		return false;

	m_bInit = true;
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void CHAJClassMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	ApplyBackgroundSchemeSettings( this, pScheme );

	m_pMapInfo->SetFgColor( pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)) );

	if ( *m_szMapName )
	{
		LoadMapPage( m_szMapName ); // reload the map description to pick up the color
	}
}

//-----------------------------------------------------------------------------
// Purpose: The CS background is painted by image panels, so we should do nothing
//-----------------------------------------------------------------------------
void CHAJClassMenu::PaintBackground()
{
}

/*

*/

void CHAJClassMenu::SetupTeam()
{

	if( GetTeam() == TEAM_AXIS )
	{
		LoadControlSettings("Resource/UI/axis_classmenu.res");
	}
	else
	{
		LoadControlSettings("Resource/UI/cwealth_classmenu.res");
	}

	m_backgroundLayoutFinished = false;
	MoveToCenterOfScreen();
	//InvalidateLayout( );
}

//-----------------------------------------------------------------------------
// Purpose: Scale / center the window
//-----------------------------------------------------------------------------
void CHAJClassMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	// stretch the window to fullscreen
	if ( !m_backgroundLayoutFinished )
		LayoutBackgroundPanel( this );
	m_backgroundLayoutFinished = true;
}

//-----------------------------------------------------------------------------
// Purpose: shows the vlass menu
//-----------------------------------------------------------------------------
void CHAJClassMenu::ShowPanel(bool bShow)
{
	// HaJ: If we're looking at the scoreboard, we shouldn't be able to open the class menu
	IViewPortPanel *pScoreboardPanel = m_pViewPort->FindPanelByName( PANEL_SCOREBOARD );
	if( bShow && pScoreboardPanel && pScoreboardPanel->IsVisible( ) == true )
	{
		Msg( "Scoreboard is open, not drawing class menu\n" );
		SetVisible( false );
		SetMouseInputEnabled( false );
		m_pViewPort->ShowBackGround( false );

		return;
	}

	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		m_bNeedsUpdate = true;

		Update();

		Activate();
		SetMouseInputEnabled( true );

		// get key bindings if shown
		if( m_iJumpKey < 0 ) // you need to lookup the jump key AFTER the engine has loaded
			m_iJumpKey = gameuifuncs->GetEngineKeyCodeForBind( "jump" );
	
		if ( m_iScoreBoardKey < 0 ) 
			m_iScoreBoardKey = gameuifuncs->GetEngineKeyCodeForBind( "showscores" );

	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}

	m_pViewPort->ShowBackGround( bShow );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJClassMenu::SetVisible( bool state )
{
	BaseClass::SetVisible ( state );

	if ( state )
	    engine->ServerCmd( "menuopen" );
    else
        engine->ServerCmd( "menuclosed" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJClassMenu::OnCommand( const char *command )
{
	gViewPortInterface->ShowPanel(PANEL_TEAM, false ); //stops never ending cycle of doom
	
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
		gViewPortInterface->ShowPanel(PANEL_TEAM, false ); //stops never ending cycle of doom
	}
	else if ( !Q_stricmp( command, "joinclass 1" ) ) // Allies
	{
		engine->ClientCmd( "jointeam 1" );
		engine->ClientCmd( const_cast<char *>( command ) );
	}
	else if ( !Q_stricmp( command, "joinclass 2" ) ) // Allies
	{
		engine->ClientCmd( "jointeam 2" );
		engine->ClientCmd( const_cast<char *>( command ) );
	}
	else if ( !Q_stricmp( command, "joinclass 3" ) ) // Allies
	{
		engine->ClientCmd( "jointeam 3" );
		engine->ClientCmd( const_cast<char *>( command ) );
	}

	SetVisible( false );
	Close();
  	gViewPortInterface->ShowBackGround( false );
	gViewPortInterface->ShowPanel( GetName(), false);
	
	BaseClass::OnCommand(command);

}

//-----------------------------------------------------------------------------
// Purpose: updates the UI with a new map name and map html page, and sets up the team buttons
//-----------------------------------------------------------------------------
void CHAJClassMenu::Update()
{

	char mapName[MAX_MAP_NAME];

	Q_FileBase( engine->GetLevelName(), mapName, sizeof(mapName) );

	SetLabelText( "mapName", mapName );
	SetProportional( true );

	/*if( NeedsUpdate() )
	{
		SetupTeam();
	}*/

	LoadMapPage( mapName );
	SetupTeam();

	//InvalidateLayout();
	
	//int x,w,h;
	
	//GetBounds(x,x,w,h);
	//SetPos( ( ScreenWidth()/2 ) - (w/2),(ScreenHeight()-h)/2);
	//m_backgroundLayoutFinished = false;

	Repaint();
	MoveToCenterOfScreen();

}
//-----------------------------------------------------------------------------
// Purpose: chooses and loads the text page to display that describes ClassName map
//-----------------------------------------------------------------------------
void CHAJClassMenu::LoadMapPage( const char *mapName )
{
	// Save off the map name so we can re-load the page in ApplySchemeSettings().
	Q_strncpy( m_szMapName, mapName, strlen( mapName ) + 1 );
	
	char mapRES[ MAX_PATH ];

	Q_snprintf( mapRES, sizeof( mapRES ), "scripts/%s.txt", mapName);

	// if no map specific description exists, load default text
	if( !vgui::filesystem()->FileExists( mapRES ) )
	{
		if( GetTeam() == TEAM_CWEALTH )
		{
			if ( vgui::filesystem()->FileExists( "resource/ui/cwealth_classes.txt" ) )
				Q_snprintf ( mapRES, sizeof( mapRES ), "resource/ui/cwealth_classes.txt");
		}
		else if( GetTeam() == TEAM_AXIS )
		{
			if ( vgui::filesystem()->FileExists( "resource/ui/axis_classes.txt" ) )
				Q_snprintf ( mapRES, sizeof( mapRES ), "resource/ui/axis_classes.txt");
		}
		else
			return; 
	}

	FileHandle_t f = vgui::filesystem()->Open( mapRES, "r" );

	// read into a memory block
	int fileSize = vgui::filesystem()->Size(f) ;
	wchar_t *memBlock = (wchar_t *)malloc(fileSize + sizeof(wchar_t));
	memset( memBlock, 0x0, fileSize + sizeof(wchar_t));
	vgui::filesystem()->Read(memBlock, fileSize, f);

	// null-terminate the stream
	memBlock[fileSize / sizeof(wchar_t)] = 0x0000;

	// check the first character, make sure this a little-endian unicode file
	if (memBlock[0] != 0xFEFF)
		m_pMapInfo->SetText( reinterpret_cast<char *>( memBlock ) ); 		// its a ascii char file
	else
		m_pMapInfo->SetText( memBlock+1 );

	// go back to the top of the text buffer
	m_pMapInfo->GotoTextStart();

	vgui::filesystem()->Close( f );
	free(memBlock);

}

void CHAJClassMenu::OnKeyCodePressed(KeyCode code)
{
	if( code == KEY_ESCAPE )
	{
		SetVisible( false ); // close menu when escape is pressed
		Close();
  		gViewPortInterface->ShowBackGround( false );
		gViewPortInterface->ShowPanel( GetName(), false);

		return;
	}

	int lastPressedEngineKey = engine->GetLastPressedEngineKey();

   if( m_iJumpKey >= 0 && m_iJumpKey == lastPressedEngineKey )
   {
      // AutoAssign();
   
   } else if ( m_iScoreBoardKey >= 0 && m_iScoreBoardKey == lastPressedEngineKey )
   {
      gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
   
   } else  {
      BaseClass::OnKeyCodePressed( code );
   }
}

void CHAJClassMenu::FireGameEvent( IGameEvent *pEvent )
{

}

