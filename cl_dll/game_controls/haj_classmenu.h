#ifndef HAJCLASSMENU_H
#define HAJCLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

#include <cl_dll/iviewport.h>

#include "igameevents.h"
#include <vgui/KeyCode.h>
#include <UtlVector.h>

#include "haj_gamerules.h"

namespace vgui
{
   class RichText;
   class HTML;
}
class TeamFortressViewport;


//-----------------------------------------------------------------------------
// Purpose: Displays the class menu
//-----------------------------------------------------------------------------
class CHAJClassMenu : public vgui::Frame, public IViewPortPanel
{
private:
   DECLARE_CLASS_SIMPLE( CHAJClassMenu, vgui::Frame );
   int m_iLastTeam;
	bool m_bNeedsUpdate;

public:
   CHAJClassMenu(IViewPort *pViewPort);
   CHAJClassMenu(IViewPort *pViewPort, const char *panelName );
   virtual ~CHAJClassMenu();

   virtual const char *GetName( void ) { return "hajclass_old"; }
   virtual void SetData(KeyValues *data) {};
   virtual void Reset() {};
   virtual void Update();
   virtual bool NeedsUpdate( void );
   virtual bool HasInputElements( void ) { return true; }
   virtual void ShowPanel( bool bShow );
   virtual void SetVisible( bool state );

   virtual const int	GetTeam() { return m_iLastTeam; }

   virtual void FireGameEvent( IGameEvent *pEvent );

   virtual void SetupTeam( void );

   // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
   vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
   virtual bool IsVisible() { return BaseClass::IsVisible(); }
   virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
   void OnCommand( const char *command );

public:
   
	void AutoAssign() {};
	virtual void PaintBackground();
	virtual void PerformLayout();
	bool m_backgroundLayoutFinished;
   
protected:
   
   // VGUI2 overrides
   virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
   virtual void OnKeyCodePressed(vgui::KeyCode code);

   // helper functions
   virtual void SetLabelText(const char *textEntryName, const char *text) {};
   virtual void LoadMapPage( const char *mapName );

   // command callbacks

   IViewPort   *m_pViewPort;
	vgui::RichText *m_pMapInfo;
   int m_iJumpKey;
   int m_iScoreBoardKey;

   bool m_bInit;

   float m_flNextTeamCheck;

	char m_szMapName[ MAX_PATH ];
};

//-----------------------------------------------------------------------------
// Purpose: Displays the class menu
//-----------------------------------------------------------------------------
class CHAJAxisClassMenu : public CHAJClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CHAJClassMenu, CHAJClassMenu );

public:
	CHAJAxisClassMenu::CHAJAxisClassMenu(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_HAJCLASS_AXIS )
	{

	}

	virtual ~CHAJAxisClassMenu() {};

	virtual const char *GetName( void ) { return PANEL_HAJCLASS_AXIS; }
	virtual const int GetTeam() { return TEAM_AXIS; };
};

class CHAJCwealthClassMenu : public CHAJClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CHAJCwealthClassMenu, CHAJClassMenu );

public:
	CHAJCwealthClassMenu::CHAJCwealthClassMenu(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_HAJCLASS_CWEALTH )
	{

	}
	virtual ~CHAJCwealthClassMenu() {};

	virtual const char *GetName( void ) { return PANEL_HAJCLASS_CWEALTH; }
	virtual const int GetTeam() { return TEAM_CWEALTH; };

};

#endif // HAJCLASSMENU_H