// Ham and Jam version indicator

#ifndef VERSIONPANEL_H
#define VERSIONPANEL_H

#include "cbase.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>

class CVersionPanel : public vgui::EditablePanel
{
	typedef vgui::EditablePanel BaseClass;

public:
	CVersionPanel( vgui::VPANEL parent );
	~CVersionPanel();

	void ParseVersionFile( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	KeyValues *m_pKV;
	vgui::Label *m_pVersionLabel;
	vgui::Label *m_pVersionDescription;
};

#endif