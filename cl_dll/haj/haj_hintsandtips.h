#ifndef HINTS_AND_TIPS_H
#define HINTS_AND_TIPS_H
#pragma once

#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "filesystem.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

#include "hudelement.h"

enum 
{
	HINT_ALIGN_LEFT = 0,
	HINT_ALIGN_RIGHT,
	HINT_ALIGN_CENTER
};

// struct for hints
struct gamehint
{
	const char* szMaterialFile;
	const char* szBindKey;
	int iShowOnce;
	int iAlwaysShow;
};

using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Displays various game hints
//-----------------------------------------------------------------------------
class CHudHintsPanel : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintsPanel, vgui::Panel );

public:
	CHudHintsPanel( const char *pElementName );

	virtual void	Init( void );
	virtual void	Reset( void );
	virtual void	OnThink( void );
	bool			ShouldDraw( void );

	void			MsgFunc_HajPlayerHint( bf_read &msg );

	void			ShowHint( const char* langKey );

	virtual void	ApplySchemeSettings( IScheme *pScheme );
	bool			LessFunc( const char& lhs, const char& rhs ) { return false; }

	// hint file parsing
	virtual void	ParseHintsFile( void );
	virtual void	AddNewHint( const char* langKey, gamehint hint )
	{
		m_Hints.Insert( langKey, hint );
	}

protected:
	virtual void	PaintBackground();
	virtual void	Paint();

private:
	Label				*m_pHintLabel;
	CBitmapImagePanel	*m_pImagePanel;

	CPanelAnimationVar( float, m_flAlpha, "alpha", "255" ); // should only be controlled from animation, not hudlayout
	CPanelAnimationVar( Color, m_BackgroundColour, "background_colour", "0 0 0 150" );
	CPanelAnimationVar( Color, m_HintDrawColour, "hint_colour", "255 255 255 200" );

	// image
	CPanelAnimationVarAliasType( float, m_iImageX, "image_x", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iImageY, "image_y", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iImageWidth, "image_wide", "40", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iImageHeight, "image_tall", "40", "proportional_float" );

	// label
	CPanelAnimationVarAliasType( float, m_iLabelX, "label_x", "55", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iLabelY, "label_y", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iLabelWidth, "label_wide", "300", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iLabelHeight, "label_tall", "45", "proportional_float" );
	CPanelAnimationVar( vgui::HFont, m_LabelFont, "label_font", "GeneralFont" );

	// other
	float m_flHintTime;
	bool m_bShowingHint;

	CUtlVector< const char* > m_ShownHints; // hints we've seen already
	CUtlDict< gamehint, int > m_Hints;
};

#endif