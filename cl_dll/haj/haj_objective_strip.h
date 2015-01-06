//========= Copyright © 2011, Ham and Jam. ==============================//
// Purpose: HUD element to show the objectives
//
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_OBJECTIVESTRIP_H
#define HAJ_OBJECTIVESTRIP_H

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

class CHudElement;
class C_HajObjective;

//-----------------------------------------------------------------------------
// Purpose: HUD element to show map's objectives
//-----------------------------------------------------------------------------
class CHudObjectiveStrip : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudObjectiveStrip, vgui::Panel );

public:
	CHudObjectiveStrip( const char *pElementName );

	virtual void Think();
	virtual void PaintBackground();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint();

	void		SetHighlightedObjective( C_HajObjective* pObj ) { m_HighlightedObjective.Set( pObj ); }

	virtual bool ShouldDraw();

private:

	int GetFlagTextureId( C_HajObjective *pObjective, int teamId);
	int FindOrCreateTexID( const char* texname );

	void DrawRoundTimer( int iBgX );
	void DrawObjectiveExtras( C_HajObjective *pObjective, int iXPos );
	void DrawExtendedInfo( C_HajObjective *pObjective, int iXPos );

	CUtlMap<int, CHandle<C_HajObjective> > m_sortedObjectives;
	CHandle<C_HajPlayer> m_LocalPlayer;

	CHandle<C_HajObjective> m_HighlightedObjective;

	CHudTexture *m_pPanelBackground;
	CHudTexture *m_pAlertIcon;
	int m_iStatusIcons[OBJECTIVE_ROLE_UNKNOWN+1];

	CPanelAnimationVarAliasType( int, m_iBackgroundHeight, "BackgroundHeight", "38", "proportional_int" );

	CPanelAnimationVarAliasType( int, m_iIconInsetX, "IconInset", "100", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconY, "IconY", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconWidth, "IconWidth", "32", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconSpacing, "IconSpacing", "3", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iStatusIconSize,"StatusIconSize", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iStatusIconX,"StatusIconX", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iStatusIconY,"StatusIconY", "0", "proportional_int" );

	CPanelAnimationVarAliasType( int, m_iTooltipY, "TooltipY", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTooltipInfoOffsetY, "TooltipInfoOffsetY", "-3", "proportional_int" );
	CPanelAnimationVar( vgui::HFont, m_TooltipCapNameFont, "TooltipTitleFont", "CaptureTitle" );
	CPanelAnimationVar( vgui::HFont, m_TooltipInfoFont, "TooltipInfoFont", "CaptureDetail" );
	CPanelAnimationVarAliasType( int, m_iTooltipInset, "TooltipInset", "2", "proportional_int" );
	CPanelAnimationVar( Color, m_TooltipBackground, "TooltipBackgroundColor", "0 0 0 50" );
	CPanelAnimationVar( Color, m_TooltipInfoColor, "TooltipInfoColor", "255 255 255 255" );

	CPanelAnimationVarAliasType( int, m_iTimerX, "RoundTimeX", "200", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTimerY, "RoundTimeY", "9", "proportional_int" );
	CPanelAnimationVar( vgui::HFont, m_hTimeFont, "RoundTimeFont", "TimeFont" );

};


#endif