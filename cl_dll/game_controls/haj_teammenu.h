//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HAJ_TEAMMENU_H
#define HAJ_TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>

#include "vgui_LocalImageButton.h"

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
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CTeamMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CTeamMenu, vgui::Frame );

public:
	CTeamMenu(IViewPort *pViewPort);
	virtual ~CTeamMenu();

	virtual const char *GetName( void ) { return PANEL_TEAM; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update();
	virtual void Think();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	virtual void SetVisible( bool state );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	// for models on panel
	virtual Panel *CreateControlByName(const char *controlName);

public:
	
	void AutoAssign();
	virtual void PaintBackground();
	virtual void PerformLayout();
	bool m_backgroundLayoutFinished;

protected:

	// VGUI2 overrides

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// helper functions
	virtual void SetLabelText( const char *textEntryName, const char *text );
	virtual void LoadMapPage( const char *mapName );
	
	// command callbacks
	virtual void OnCommand( const char *command);

	vgui::Label *m_pTeamInfo;
	IViewPort	*m_pViewPort;
	vgui::RichText *m_pMapInfo;
	vgui::HTML *m_pMapInfoHTML;

	vgui::LocalImageButton *m_pCwealthPropaganda;
	vgui::LocalImageButton *m_pAxisPropaganda;
	vgui::LocalImageButton *m_pTeamFull;

	//vgui::LocalImageButton *m_pCwealthConfirm;
	//vgui::LocalImageButton *m_pAxisConfirm;
	vgui::LocalImageButton *m_pCancelButton;
	vgui::LocalImageButton *m_pAutoAssignButton;
	vgui::LocalImageButton *m_pSpectateButton;

	vgui::Label *m_pTitle;

	bool m_bAxisFull;
	bool m_bCWealthFull;

	int m_iJumpKey;
	int m_iScoreBoardKey;
	int m_iActiveTeamInfo;

	char m_szMapName[ MAX_PATH ];
	KeyValues *m_pMapSettings;

	CPanelAnimationVarAliasType( int, m_iTeamFullX, "TeamFullX", "95", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTeamFullY, "TeamFullY", "190", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTeamFullW, "TeamFullW", "240", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTeamFullH, "TeamFullH", "60", "proportional_int" );
};

#endif // HAJ_TEAMMENU_H
