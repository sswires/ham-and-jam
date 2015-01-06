#ifndef VOICEMENU_H
#define VOICEMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

#include <cl_dll/iviewport.h>

#include <vgui/KeyCode.h>
#include <UtlVector.h>

namespace vgui
{
   class RichText;
   class HTML;
}
class TeamFortressViewport;


//-----------------------------------------------------------------------------
// Purpose: Displays the class menu
//-----------------------------------------------------------------------------
class CVoiceMenu : public vgui::Frame, public IViewPortPanel
{
private:
   DECLARE_CLASS_SIMPLE( CVoiceMenu, vgui::Frame );

public:
   CVoiceMenu(IViewPort *pViewPort);
   virtual ~CVoiceMenu();

   virtual const char *GetName( void ) { return PANEL_VOICE; }
   virtual void SetData(KeyValues *data) {};
   virtual void Reset() {};
   virtual void Update() {};
   virtual bool NeedsUpdate( void ) { return false; }
   virtual bool HasInputElements( void ) { return true; }
   virtual void ShowPanel( bool bShow );

   // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
   vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
     virtual bool IsVisible() { return BaseClass::IsVisible(); }
   virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
   void OnCommand( const char *command );
public:
   
   void AutoAssign() {};
   
protected:
   
   // VGUI2 overrides
   virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
   virtual void OnKeyCodePressed(vgui::KeyCode code);

   // helper functions
   virtual void SetLabelText(const char *textEntryName, const char *text) {};


   // command callbacks

   IViewPort   *m_pViewPort;

   int m_iJumpKey;
   int m_iScoreBoardKey;
   int m_inum1;
   int m_inum2;
   int m_inum3;
   int m_inum4;
   int m_inum5;
   int m_inum6;
   int m_inum7;
   int m_inum8;
   int m_inum9;
   int m_inum0;
};


#endif // CLASSMENU_H