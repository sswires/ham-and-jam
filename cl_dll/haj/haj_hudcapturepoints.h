
// haj_hudcapturepoints.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_HUDCAPTUREPOINTS
#define __INC_HUDCAPTUREPOINTS

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef HUDELEMENT_H
	#include "hudelement.h"
#endif
#ifndef PANEL_H
	#include "vgui_controls/Panel.h"
#endif

#include "haj_capturepoint_c.h"

class C_HajObjective;

/////////////////////////////////////////////////////////////////////////////
class CHudCapturePoint : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudCapturePoint, vgui::Panel);

public:
	// 'structors
	CHudCapturePoint(const char* szElementName);
	virtual ~CHudCapturePoint();

public:
	// CHudElement overrides
	virtual void Init();
	virtual void Reset();
	virtual void VidInit();

	// vgui::Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual bool ShouldDraw( void );
	virtual void OnThink();
	virtual void Paint();

	C_HajObjective *m_pHighlightedObjective;

private:
	int GetFlagTextureId( C_HajObjective *pObjective, int teamId);
	int FindOrCreateTexID( char* texname );

	CPanelAnimationVarAliasType( int, m_iBackgroundMargin, "background_margin", "2", "proportional_int" );
	CHudTexture		*m_pIconBackground;		// background for each cap icon
	CHudTexture		*m_pAlertIcon;			// red obj_flash alert for when capping

private:
	int				m_iLockedIconID;	// "locked" icon texture ID
	int				m_iDefendIconID;
	int				m_iDestroyedIconID;
	int				m_iDefuseIconID;
	int				m_iPlantIconID;

	vgui::HFont		m_hTitleFont;
	vgui::HFont		m_hDetailFont;
	vgui::HFont		m_hPlayerFont;
};

/////////////////////////////////////////////////////////////////////////////
#endif
