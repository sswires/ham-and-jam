/*
	© 2010 Ham and Jam Team
	============================================================
	Author: Stephen Swires
	Purpose: Image button that supports localization
*/

#include "cbase.h"

#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IImage.h>
#include <FileSystem.h>

#include <cl_dll/iviewport.h>

#include "vgui_LocalImageButton.h"

vgui::LocalImageButton::LocalImageButton( Panel *parent, const char* panelName, const char* image, const char* command, bool bCreateAuxPanel ) : ImagePanel( parent, panelName )
{
	SetParent( parent );

	m_bCommand = false;

	if( bCreateAuxPanel )
	{
		m_pAuxImage = new vgui::ImagePanel( this, "AuxImage" );

		m_pAuxImage->SetVisible( false );
		m_pAuxImage->SetShouldScaleImage( true );
		m_pAuxImage->SetMouseInputEnabled( false );
		m_pAuxImage->SetKeyBoardInputEnabled( false );
	}
	else
		m_pAuxImage = NULL;

	// button command
	if( command != NULL )
	{
		Q_strcpy( m_szCommand, command );
		m_bCommand = true;
	}

	m_szMouseOutCommand[0] = '\0';
	m_szMouseOverCommand[0] = '\0';

	if( image != NULL )
	{
		SetLocalImage( image );
	}

}

void vgui::LocalImageButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void vgui::LocalImageButton::PerformLayout()
{
	BaseClass::PerformLayout();

	// setup auxilery image
	if( m_pAuxImage != NULL )
	{
		m_pAuxImage->SetBounds( m_iAuxImagePosX, m_iAuxImagePosY, m_iAuxImageWide, m_iAuxImageHigh );
		IImage *pPanelImage = m_pAuxImage->GetImage();

		if( pPanelImage )
			pPanelImage->SetColor( m_AuxDimAlpha );
	}

	// set colour of current image
	IImage *pPanelImage = GetImage();
	
	if( pPanelImage != NULL )
	{
		pPanelImage->SetColor( m_DimmedAlpha );
	}
}

void vgui::LocalImageButton::SetLocalImage( const char* image )
{
	SetLocalImage( this, image );
}

void vgui::LocalImageButton::SetLocalAuxImage( const char* image )
{
	if( m_pAuxImage )
	{
		SetLocalImage( m_pAuxImage, image );
		IImage *pPanelImage = m_pAuxImage->GetImage();

		if( pPanelImage != NULL )
		{
			m_pAuxImage->SetVisible( true );
			pPanelImage->SetColor( m_AuxDimAlpha );
		}
	}
}

bool vgui::LocalImageButton::CheckImageExists( const char *image )
{
	char fullPath[MAX_PATH];
	Q_snprintf( fullPath, MAX_PATH, "materials/VGUI/%s.vmt", image );

	return vgui::filesystem()->FileExists( fullPath );
}

void vgui::LocalImageButton::SetLocalImage( vgui::ImagePanel *control, const char* image )
{
	char curFilename[MAX_PATH];
	char uilanguage[ 64 ];

	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	Q_snprintf( curFilename, MAX_PATH, "%s_%s", image, uilanguage );

	// language specific
	if( CheckImageExists( curFilename ) )
	{
		control->SetImage( curFilename );
		return;
	}

	// english fallback
	Q_snprintf( curFilename, MAX_PATH, "%s_english", image );

	if( CheckImageExists( curFilename ) )
	{
		control->SetImage( curFilename );
		return;
	}

	// nope, really nope
	control->SetImage( image );
}

void vgui::LocalImageButton::OnCursorEntered()
{
	if( GetImage() )
		GetImage()->SetColor( m_LitAlpha );

	if( m_pAuxImage && m_pAuxImage->IsVisible() && m_pAuxImage->GetImage() )
		m_pAuxImage->GetImage()->SetColor( m_AuxLitAlpha );

	// mouse over command
	if( m_szMouseOverCommand[0] != '\0' )
	{
		GetParent()->OnCommand( m_szMouseOverCommand );
	}
}

void vgui::LocalImageButton::OnCursorExited()
{
	if( GetImage() )
		GetImage()->SetColor( m_DimmedAlpha );

	if( m_pAuxImage && m_pAuxImage->IsVisible() && m_pAuxImage->GetImage() )
		m_pAuxImage->GetImage()->SetColor( m_AuxDimAlpha );

	// mouse out command
	if( m_szMouseOutCommand[0] != '\0' )
	{
		GetParent()->OnCommand( m_szMouseOutCommand );
	}
}

void vgui::LocalImageButton::OnMousePressed( MouseCode code )
{
	if( m_bCommand )
		GetParent()->OnCommand( m_szCommand );
}