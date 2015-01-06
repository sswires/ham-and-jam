//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: The Ham and Jam logo on game menu
// Note:	
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_vgui_menu_logo.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: transform a normalized value into one that is scaled based the minimum
//          of the horizontal and vertical ratios
//-----------------------------------------------------------------------------
static int GetAlternateProportionalValueFromNormal(int normalizedValue)
{
	int wide, tall;
	GetHudSize( wide, tall );
	int proH, proW;
	surface()->GetProportionalBase( proW, proH );
	double scaleH = (double)tall / (double)proH;
	double scaleW = (double)wide / (double)proW;
	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	return (int)( normalizedValue * scale );
}

//-----------------------------------------------------------------------------
// Purpose: Displays the logo panel
//-----------------------------------------------------------------------------
class CMenuLogo : public vgui::EditablePanel
{
	typedef vgui::EditablePanel BaseClass;

public:
	
	CMenuLogo( vgui::VPANEL parent );
	~CMenuLogo();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PaintBackground();
	virtual void PerformLayout();

private:
	
	ImagePanel *pLogoPanel;
	
	int logoHeight;
	int logoWidth;
	int logoInsetX;
	int logoInsetY;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMenuLogo::CMenuLogo( vgui::VPANEL parent ) : BaseClass( NULL, "CMenuLogo" )
{
	SetParent( parent );
	SetProportional( true );
	SetVisible( true );

	SetScheme( "SourceScheme" );

	// Loading the .res file.
	//LoadControlSettings( "resource/UI/MenuLogo.res" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	// create a new image for our logo
	pLogoPanel = new ImagePanel( this, "Logo");
	if ( pLogoPanel )
	{
		pLogoPanel->SetShouldScaleImage( true );
		pLogoPanel->SetImage( "menulogo");
		pLogoPanel->SetBounds( 0, 0, 0, 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMenuLogo::~CMenuLogo()
{
	if ( pLogoPanel )
		delete pLogoPanel;
	pLogoPanel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Apply Scheme Settings
//-----------------------------------------------------------------------------
void CMenuLogo::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	// set the background colour to 100% transparent
	SetBgColor( Color( 0,0,0,0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: Paint Background
//-----------------------------------------------------------------------------
void CMenuLogo::PaintBackground()
{
	SetPaintBackgroundType( 0 );
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: Paint Background
//-----------------------------------------------------------------------------
void CMenuLogo::PerformLayout()
{
	BaseClass::PerformLayout();
	
	int wide, tall;
	GetHudSize( wide, tall );

	// calculate our width, height and offsets based on screen resolution
	logoHeight = GetAlternateProportionalValueFromNormal( 128 );
	logoWidth = GetAlternateProportionalValueFromNormal( 256 );
	logoInsetX = wide - logoWidth;
	logoInsetY = 0;

	SetBounds( logoInsetX, logoInsetY, logoWidth, logoHeight);

	// resize our image to match our panel
	if ( pLogoPanel )
	{
		pLogoPanel->SetBounds( 0, 0, logoWidth, logoHeight  );
	}
}

// Class
class CTop : public ITop
{
private:
	CMenuLogo *MenuLogo;
	vgui::VPANEL m_hParent;

public:
	CTop( void )
	{
		MenuLogo = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		// Create immediately
		MenuLogo = new CMenuLogo(parent);
	}

	void Destroy( void )
	{
		if ( MenuLogo )
		{
			MenuLogo->SetParent( (vgui::Panel *)NULL );
			delete MenuLogo;
		}
	}

};

static CTop g_Top;
ITop *Top = ( ITop * )&g_Top;