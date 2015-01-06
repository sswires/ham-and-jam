//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose: The team header section of the scoreboard.
// Note:	
// $NoKeywords: $
//=======================================================================//

#include <vgui/VGUI.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>
#include <igameevents.h>

class CScoreboardTeamHeader : public vgui::EditablePanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CScoreboardTeamHeader, vgui::EditablePanel );

public:
	CScoreboardTeamHeader( vgui::VPANEL parent );
	CScoreboardTeamHeader( vgui::Panel *parent, const char* panelName );
	~CScoreboardTeamHeader();

	int GetTeam( void ) { return m_iHeaderTeam; }
	int SetTeam( int i ) { m_iHeaderTeam = i; }

	virtual void InitControls( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	virtual void Think( void );
	virtual void Paint( void );
	virtual void PaintBackground( void );
	virtual void PaintBorder( void );

	virtual void FireGameEvent( IGameEvent *event );


protected:

	CPanelAnimationVar( int, m_iHeaderTeam, "Team", "0" );

	CPanelAnimationVar( Color, m_BorderColor, "BorderColor", "255 255 255 100" );
	CPanelAnimationVar( Color, m_ScoreColor, "ScoreColor", "255 255 255 255" );

	CPanelAnimationVar( vgui::HFont, m_TeamNameFont, "TeamNameFont", "ScoreboardTeam" );
	CPanelAnimationVar( vgui::HFont, m_UnitNameFont, "UnitNameFont", "CaptureDetail" );
	CPanelAnimationVar( vgui::HFont, m_PlayerCountFont, "PlayerCountFont", "GeneralFont" );
	CPanelAnimationVar( vgui::HFont, m_ScoreFont, "ScoreFont", "TimeFont" );

	CPanelAnimationVarAliasType( int, m_iTopPadding, "TopPadding", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iUnitSpacing, "UnitY", "25", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTeamNameX, "TeamNameX", "47", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iScorePadding, "ScorePadding", "25", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconPadding, "IconPadding", "5", "proportional_int" );
	CPanelAnimationVar( float, m_flIconAspect, "IconAspect", "1.64" );

	vgui::Label *m_pTeamNameLabel;
	vgui::Label *m_pUnitNameLabel;
	vgui::Label *m_pPlayerCountLabel;
	vgui::Label *m_pScoreLabel;

	CHudTexture		*m_pFlag;
	CHudTexture		*m_pFlagGerman;				// flags
	CHudTexture		*m_pFlagBritish;
	CHudTexture		*m_pFlagPolish;
	CHudTexture		*m_pFlagCanadian;

};