//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MPCLIENTSCOREBOARDDIALOG_H
#define HL2MPCLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>
#include "haj_scoreboard_team.h"

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CHL2MPClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE( CHL2MPClientScoreBoardDialog, CClientScoreBoardDialog );

public:
	CHL2MPClientScoreBoardDialog( IViewPort *pViewPort );
	~CHL2MPClientScoreBoardDialog();

	virtual void Reset();
	virtual void Update();

	// vgui overrides for rounded corner background
	virtual void Paint();
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	void UpdateItemVisibiity();
	void InitPlayerList( vgui::SectionedListPanel *pPlayerList, int teamNumber );
	void UpdateTeamInfo();
	void UpdatePlayerList();
	void UpdateSpectatorList();
	bool GetPlayerScoreInfo( int playerIndex, KeyValues *outPlayerInfo );

	bool ShouldShowAsSpectator( int iPlayerIndex );
	void FireGameEvent( IGameEvent *event );

	static bool HL2MPPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 );

	void SetLabelText(const char *textEntryName, wchar_t *text);

	// rounded corners
	Color					m_bgColor;
	Color					m_borderColor;

	int						m_iStoredScoreboardWidth; // Store the full scoreboard width.

	int						m_iLocalPlayerItemID;

	// player lists
	vgui::SectionedListPanel *m_pPlayerListCommonwealth;
	vgui::SectionedListPanel *m_pPlayerListAxis;

	vgui::Label *m_pHostname;
	vgui::Label *m_pSpectators;

	vgui::Label	*m_pPlayerCountLabel_Commonwealth;
	vgui::Label	*m_pScoreHeader_Commonwealth;
	vgui::Label	*m_pScoreLabel_Commonwealth;
	vgui::Label	*m_pDeathsHeader_Commonwealth;
	vgui::Label	*m_pPingHeader_Commonwealth;
	vgui::Label	*m_pPingLabel_Commonwealth;

	vgui::Label	*m_pPlayerCountLabel_Axis;
	vgui::Label	*m_pScoreHeader_Axis;
	vgui::Label	*m_pScoreLabel_Axis;
	vgui::Label	*m_pDeathsHeader_Axis;
	vgui::Label	*m_pPingHeader_Axis;
	vgui::Label	*m_pPingLabel_Axis;

	CScoreboardTeamHeader *m_pCwealthHeader;
	CScoreboardTeamHeader *m_pAxisHeader;

	// Create the vertical line so we can hide it in single column mode.
	vgui::ImagePanel *m_pVertLine;

	CPanelAnimationVarAliasType( int, m_iObjectivesWidth, "objectives_width", "35", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCEWidth, "ce_width", "35", "proportional_int" );

	CPanelAnimationVar( Color, m_HighlightColor, "highlightColor", "200 200 200 50" );
	CPanelAnimationVar( vgui::HFont, m_hLabelFont, "LabelFont", "Default" );
};


#endif // HL2MPCLIENTSCOREBOARDDIALOG_H
