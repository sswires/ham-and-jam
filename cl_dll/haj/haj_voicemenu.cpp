#include "cbase.h"
#include <cdll_client_int.h>

#include "haj_voicemenu.h"

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
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include <cl_dll/iviewport.h>

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//void UpdateCursorState();

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CVoiceMenu::CVoiceMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_VOICE )
{
   m_pViewPort = pViewPort;
   m_iJumpKey = -1; // this is looked up in Activate()
   m_iScoreBoardKey = -1; // this is looked up in Activate()
   m_inum1 = -1;
   m_inum2 = -1;
   m_inum3 = -1;
   m_inum4 = -1;
   m_inum5 = -1;
   m_inum6 = -1;
   m_inum7 = -1;
   m_inum8 = -1;
   m_inum9 = -1;
   m_inum0 = -1;

   // initialize dialog
   SetTitle("", true);

   // load the new scheme early!!
    SetScheme("ClientScheme");
    SetMoveable(false);
    SetSizeable(false);
	//SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( true );

   // hide the system buttons
   SetTitleBarVisible( false );
   SetProportional(true);

   LoadControlSettings("Resource/UI/voicemenu.res");
   InvalidateLayout();


}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CVoiceMenu::~CVoiceMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void CVoiceMenu::ApplySchemeSettings(IScheme *pScheme)
{
   BaseClass::ApplySchemeSettings(pScheme);
   	SetPaintBackgroundType( 2 );
	BaseClass::PaintBackground();
	//SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( true );
}

//-----------------------------------------------------------------------------
// Purpose: shows the voice menu
//-----------------------------------------------------------------------------
void CVoiceMenu::ShowPanel(bool bShow)
{
   if ( BaseClass::IsVisible() == bShow )
      return;

   if ( bShow )
   {
      Activate();

      //SetMouseInputEnabled( false );
	  SetKeyBoardInputEnabled( true );

      // get key bindings if shown

      if( m_iJumpKey < 0 ) // you need to lookup the jump key AFTER the engine has loaded
      {
         m_iJumpKey = gameuifuncs->GetEngineKeyCodeForBind( "jump" );
      }
      if( m_inum1 < 0 ) 
      {
         m_inum1 = gameuifuncs->GetEngineKeyCodeForBind( "slot1" );
      }
      if( m_inum2 < 0 ) 
      {
         m_inum2 = gameuifuncs->GetEngineKeyCodeForBind( "slot2" );
      }
        if( m_inum3 < 0 ) 
      {
         m_inum3 = gameuifuncs->GetEngineKeyCodeForBind( "slot3" );
      }
      if( m_inum4 < 0 ) 
      {
         m_inum4 = gameuifuncs->GetEngineKeyCodeForBind( "slot4" );
      }
        if( m_inum5 < 0 ) 
      {
         m_inum5 = gameuifuncs->GetEngineKeyCodeForBind( "slot5" );
      }
      if( m_inum6 < 0 ) 
      {
         m_inum6 = gameuifuncs->GetEngineKeyCodeForBind( "slot6" );
      }

      if ( m_iScoreBoardKey < 0 )
      {
         m_iScoreBoardKey = gameuifuncs->GetEngineKeyCodeForBind( "showscores" );
      }
   }
   else
   {
      SetVisible( false );
      //SetMouseInputEnabled( false );
      SetKeyBoardInputEnabled( false );
   }

   m_pViewPort->ShowBackGround( bShow );
}

void CVoiceMenu::OnCommand( const char *command )
{
}

void CVoiceMenu::OnKeyCodePressed(KeyCode code)
{
   int lastPressedEngineKey = engine->GetLastPressedEngineKey();

   if( m_iJumpKey >= 0 && m_iJumpKey == lastPressedEngineKey )
   {
	gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
    if( m_inum1 >= 0 && m_inum1 == lastPressedEngineKey )
   {
      engine->ClientCmd("voice_go");
	  engine->ClientCmd("say_team (VOICE) Go Go Go!");
	  gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
    if( m_inum2 >= 0 && m_inum2 == lastPressedEngineKey )
   {
      engine->ClientCmd("voice_grenade");
		engine->ClientCmd("say_team (VOICE) Grenade!");
	  gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
    if( m_inum3 >= 0 && m_inum3 == lastPressedEngineKey )
   {
      engine->ClientCmd("voice_takeobj");
		engine->ClientCmd("say_team (VOICE) Take The Objective!");
	  gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
    if( m_inum4 >= 0 && m_inum4 == lastPressedEngineKey )
   {
      engine->ClientCmd("voice_sniper");
		engine->ClientCmd("say_team (VOICE) Sniper!");
	  gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
    if( m_inum5 >= 0 && m_inum5 == lastPressedEngineKey )
   {
      engine->ClientCmd("voice_hold");
		engine->ClientCmd("say_team (VOICE) Hold Your Positions!");
	  gViewPortInterface->ShowPanel( PANEL_VOICE, false );
   }
      if( m_inum6 >= 0 && m_inum6 == lastPressedEngineKey )
   {
      gViewPortInterface->ShowPanel( PANEL_VOICE, false );
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