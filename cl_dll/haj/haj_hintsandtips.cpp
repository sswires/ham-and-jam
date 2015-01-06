#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "filesystem.h"

#include "haj_player_c.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

#include "hudelement.h"

#include "haj_hintsandtips.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar haj_cl_hints_enabled( "haj_cl_hints_enabled", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar haj_cl_hints_keeptime( "haj_cl_hints_keeptime", "4", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

DECLARE_HUDELEMENT( CHudHintsPanel );

DECLARE_HUD_MESSAGE( CHudHintsPanel, HajPlayerHint );

CHudHintsPanel* g_HintsPanel = NULL;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintsPanel::CHudHintsPanel( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudHintsPanel")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetScheme("ClientScheme");
	SetProportional( true );

	m_flAlpha = 0.0f;
	m_flHintTime = 0.0f;
	m_bShowingHint = false;

	// Init sub panels
	m_pHintLabel = new Label( this, "HintLabelText", "" );
	m_pHintLabel->SetProportional( true );

	m_pImagePanel = new CBitmapImagePanel( this, "HintLabelImage" );
	m_pHintLabel->SetProportional( true );

	ParseHintsFile();

	g_HintsPanel = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::ApplySchemeSettings( IScheme *pScheme )
{
	// image settings
	m_pImagePanel->SetSize( m_iImageWidth, m_iImageHeight );
	m_pImagePanel->SetPos( m_iImageX, m_iImageY );

	// label
	m_pHintLabel->SetFont( m_LabelFont );
	m_pHintLabel->SetSize( m_iLabelWidth, m_iLabelHeight );
	m_pHintLabel->SetPos( m_iLabelX, m_iLabelY );
	m_pHintLabel->SetContentAlignment( vgui::Label::a_west );

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::Init( void )
{
	HOOK_HUD_MESSAGE( CHudHintsPanel, HajPlayerHint );
	m_bShowingHint = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::MsgFunc_HajPlayerHint( bf_read &msg )
{
	char szString[256];
	msg.ReadString( szString, sizeof(szString) );

	ShowHint( szString );
}

//-----------------------------------------------------------------------------
// Purpose: parses the hints fine containing all the game hints
//-----------------------------------------------------------------------------
void CHudHintsPanel::ParseHintsFile( void )
{
	m_Hints.RemoveAll();

	if( vgui::filesystem()->FileExists( "resource/gametips.txt" ) )
	{
		KeyValues *pKeyValues = new KeyValues( "GameTips" );
		pKeyValues->LoadFromFile( vgui::filesystem(), "resource/gametips.txt", "MOD" );
		pKeyValues->FindKey( "GameTips", false );

		if( pKeyValues )
		{
			for ( KeyValues *pKey = pKeyValues->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
			{
				gamehint thisHint;
				thisHint.iAlwaysShow = pKey->GetInt( "alwaysshow", 0 );
				thisHint.iShowOnce = pKey->GetInt( "showonce", 1 );
				thisHint.szMaterialFile = pKey->GetString( "material", "" );
				thisHint.szBindKey = pKey->GetString( "bindkey", "" );

				AddNewHint( pKey->GetName(), thisHint );
			}
		}
	}
	else
		Msg( "Warning: tips file resource/gametips.txt not found!" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudHintsPanel::ShouldDraw()
{
	if( C_HajPlayer::GetLocalHajPlayer() )
	{
		if( C_HajPlayer::GetLocalHajPlayer()->GetTeamNumber() == TEAM_SPECTATOR )
			return false;
	}

	return true;
}


void CHudHintsPanel::ShowHint( const char* langKey )
{
	int index = m_Hints.Find( langKey );

	if( index >= 0 )
	{
		gamehint curHint = m_Hints.Element( index );

		if( curHint.iShowOnce > 0 && m_ShownHints.HasElement( langKey ) )
			return;

		if( curHint.iAlwaysShow == 0 && haj_cl_hints_enabled.GetBool() == false )
			return;

		wchar_t uni[255];
		_snwprintf( uni, sizeof( uni), L"%s", vgui::localize()->Find( langKey ) );

		m_pHintLabel->SetText( uni );
		m_pImagePanel->setTexture( curHint.szMaterialFile, true );
		
		if( curHint.iShowOnce > 0 )
			m_ShownHints.AddToTail( langKey );

		m_flAlpha = 255;
		m_flHintTime = gpGlobals->curtime;
		m_bShowingHint = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::OnThink( void )
{
	if( gpGlobals->curtime >= m_flHintTime + haj_cl_hints_keeptime.GetFloat() )
	{
		m_flAlpha = clamp( m_flAlpha - ( gpGlobals->frametime * 200 ), 0, 255 );

		if( m_flAlpha <= 0 )
			m_bShowingHint = false;
	}

	SetAlpha( m_flAlpha );
	m_pImagePanel->SetAlpha( m_flAlpha );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::PaintBackground( void )
{
	surface()->DrawSetColor( m_BackgroundColour );
	surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintsPanel::Paint( void )
{

}