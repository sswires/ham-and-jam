//========= Copyright © 2011, Ham and Jam. ==============================//
// Purpose: HUD element to show extended information about an objective
//
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_OBJECTIVETOOLTIP_H
#define HAJ_OBJECTIVETOOLTIP_H

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
// Purpose: HUD element to show extended information about a capture point "in focus"
//-----------------------------------------------------------------------------
class CHudObjectiveTooltip : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudObjectiveTooltip, vgui::Panel );

public:
	CHudObjectiveTooltip( const char *pElementName );

	virtual void Paint();
	virtual bool ShouldDraw();

private:

	CHandle<C_HajObjective> m_hObjectiveInFocus;
	bool m_bPlayerInObjective; // true when player is in area, so objective overlay doesn't override the in focus obj


};


#endif