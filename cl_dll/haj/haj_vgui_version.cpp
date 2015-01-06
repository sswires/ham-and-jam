// Ham and Jam version indicator
#include "cbase.h"
#include "vgui_controls/Frame.h"
#include "filesystem.h"
#include <KeyValues.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

#include "haj_vgui_version.h"

CVersionPanel *g_pVersionPanel;

// @purpose: Constructor
CVersionPanel::CVersionPanel( vgui::VPANEL parent ) : BaseClass( NULL, "CVersionPanel" )
{
	SetParent( parent );
	SetProportional( true );
	SetVisible( true );

	SetScheme( "ClientScheme" );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_pVersionDescription = new vgui::Label( this, "VersionDescription", "#HaJ_ClosedBeta" );
	m_pVersionLabel = new vgui::Label( this, "VersionLabel", "#HaJ_UnknownVersion" );

	ParseVersionFile();
}

// @purpose: Destructor
CVersionPanel::~CVersionPanel()
{
	if( m_pKV )
		m_pKV->deleteThis();
}

void CVersionPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	int w, h;
	GetHudSize( w, h );

	int margin = ( 35 / (h/480) );
	SetBounds( margin, margin, w - (margin*2), h - (margin*2) );

	// Description of version. Nagging about beta version
	m_pVersionDescription->SetFont( pScheme->GetFont( "CaptureDetail", true ) );
	m_pVersionDescription->SetContentAlignment( vgui::Label::a_southeast );
	m_pVersionDescription->SizeToContents();
	m_pVersionDescription->SetPos( GetWide() - m_pVersionDescription->GetWide(), GetTall() - m_pVersionDescription->GetTall() );

	// Set up version label appearance
	m_pVersionLabel->SetFont( pScheme->GetFont("CaptureTitle", true ) );
	m_pVersionLabel->SetContentAlignment( vgui::Label::a_southeast );
	m_pVersionLabel->SetSize( GetWide(), (margin * 3) );

	if( m_pVersionDescription->GetWide() > 0 )
		m_pVersionLabel->SetPos( 0, GetTall() - m_pVersionLabel->GetTall() - (m_pVersionDescription->GetTall() + 2) );
	else
		m_pVersionLabel->SetPos( 0, GetTall() - m_pVersionLabel->GetTall() );
}

// @purpose: Parses the text file containing our current version number and check URI
void CVersionPanel::ParseVersionFile( void )
{
	m_pKV = new KeyValues( "version.txt" );

	if( !m_pKV || !m_pKV->LoadFromFile( vgui::filesystem(), "version.txt", "MOD" ) )
		Msg( "Failed to read version number from version.txt\n" );
	else
	{
		m_pVersionLabel->SetText( m_pKV->GetString( "CurrentVersion", "#HaJ_UnknownVersion" ) );
		m_pVersionDescription->SetText( m_pKV->GetString( "VersionDetail", "" ) );
	}
}