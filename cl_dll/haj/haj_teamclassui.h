#ifndef HAJ_CLASS_SELECTION_H
#define HAJ_CLASS_SELECTION_H

#ifdef _WIN32
	#pragma once
#endif

#include "cbase.h"

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/BitmapImagePanel.h>

#include <cl_dll/iviewport.h>

#include "hl2mp_player_shared.h"
#include "haj_modelpanel.h"
#include "vgui_LocalImageButton.h"

//==============================================================
// Ham and Jam class menu (new one)
//=============================================================
class CHAJClassSelection : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHAJClassSelection, vgui::Frame );

public:
	CHAJClassSelection( IViewPort *pViewPort );

	virtual void		PaintBackground();
	virtual void		PerformLayout();
	virtual void		ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void		ShowPanel( bool bShow );
	virtual const char *GetName( void ) { return PANEL_HAJCLASS; }

	void				OnCommand( const char *command );
	void				SetupTeam( int iTeam ); // setup team for panel
	void				ChangeTab( int iClass );
	void				ConfirmSelection( void );

	virtual void		SetData(KeyValues *data) {};
	virtual void		Reset() {};

	virtual void		Think();

	virtual bool		NeedsUpdate();
	virtual void		Update();

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL		GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool		IsVisible() { return BaseClass::IsVisible(); }
	virtual void		SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual bool		HasInputElements( void ) { return true; }

	void				UpdateClassBackground( int iTeam, int iClass );

private:
	int			m_iCurTeam;
	int			m_iSelectedClass;
	int			m_iTabbedToClass;

	vgui::CBitmapImagePanel *m_pBackground;
	CModelPanel *m_pModelPanel;

	vgui::Button *m_pClassTabs[CLASS_LAST];
	vgui::Label *m_pClassTitle;
	vgui::Label *m_pClassName;
	vgui::Label *m_pClassInfo;
	vgui::Label *m_pPlayerCount;
	vgui::CheckButton *m_pSuicideOnClassChange;
	
	vgui::LocalImageButton *m_pCancelButton;
	vgui::LocalImageButton *m_pConfirmButton;

	vgui::ImagePanel *m_pClassBackground;

	CHudTexture *m_Background;

	CPanelAnimationVarAliasType( int, m_iClassButtonsX, "ClassButtonsX", "50", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iClassButtonsY, "ClassButtonsY", "15", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iClassButtonsW, "ClassButtonsW", "30", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iClassButtonsH, "ClassButtonsH", "7", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iClassButtonsSpacing, "ClassButtonsSpacing", "5", "proportional_int" );

	CPanelAnimationVar( vgui::HFont, m_TitleFont, "TitleFont", "HajPanelTitles" );
	CPanelAnimationVar( vgui::HFont, m_ClassLabelFont, "ClassNameFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_ClassTabFont, "ClassTabFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_ClassDescriptionFont, "DescriptionFont", "Default" );
	CPanelAnimationVar( Color, m_DescriptionColor, "DescriptionColor", "15 15 15 200" );
	CPanelAnimationVar( Color, m_SelectColor, "SelectedTabBg", "255 255 255 50" );
};

#endif