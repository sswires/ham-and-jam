//========= Copyright © 2011, Ham and Jam. ==============================//
// Purpose: HUD element to show extended information about a tooltip
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_misc.h"
#include "haj_objectivemanager_c.h"
#include "haj_capturepoint_c.h"
#include "haj_objective_tooltip.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudObjectiveTooltip );

CHudObjectiveTooltip::CHudObjectiveTooltip( const char *pElementName ) : BaseClass(NULL, "HudObjectiveTooltip"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled(true);

	m_hObjectiveInFocus = NULL;
	m_bPlayerInObjective = false;

}

//-----------------------------------------------------------------------------
// Purpose: Draw the objective info then - null check is done in ShouldDraw
//-----------------------------------------------------------------------------
void CHudObjectiveTooltip::Paint()
{

}

//-----------------------------------------------------------------------------
// Purpose: Draw this panel?
//-----------------------------------------------------------------------------
bool CHudObjectiveTooltip::ShouldDraw()
{
	if( m_hObjectiveInFocus == NULL )
		return false;

	return CHudElement::ShouldDraw();
}