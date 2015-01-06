
// haj_hudtimer.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_HUDTIMER
#define __INC_HUDTIMER

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef HUDELEMENT_H
	#include "hudelement.h"
#endif
#ifndef PANEL_H
	#include "vgui_controls/Panel.h"
#endif

#include "vgui/ISurface.h"

/////////////////////////////////////////////////////////////////////////////
class CHudTimer : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudTimer, vgui::Panel);

public:
	// 'structors
	CHudTimer(const char* szElementName);
	virtual ~CHudTimer();

public:
	// CHudElement overrides
	virtual void Init();
	virtual void Reset();
	virtual void VidInit();

	// vgui::Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual void OnThink();
	virtual void Paint();

	bool ShouldDraw();

	float GetTimeLeft( void );

private:
	// private data
	wchar_t	m_szTimeLeft[32];
	wchar_t m_szReinforcements[60];
	int		m_lenTimeLeft;			// length of m_szTimeLeft
	vgui::HFont hFont;
	vgui::HFont hTimerFont;

	CPanelAnimationVar( int, m_iDrawTime, "DrawTime", "1" );
	CPanelAnimationVarAliasType( int, m_iReinforcementPosX, "ReinsX", "5", "proportional_int" );
};

/////////////////////////////////////////////////////////////////////////////
#endif
