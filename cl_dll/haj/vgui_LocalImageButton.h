/*
	© 2010 Ham and Jam Team
	============================================================
	Author: Stephen Swires
	Purpose: Image button that supports localization
*/

#ifndef HAJ_LOCALIMAGEBUTTON_H
#define HAJ_LOCALIMAGEBUTTON_H

#include "cbase.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>

#include <cl_dll/iviewport.h>

namespace vgui
{
	class LocalImageButton : public vgui::ImagePanel
	{
		DECLARE_CLASS_SIMPLE( LocalImageButton, vgui::ImagePanel );

	public:
		LocalImageButton( Panel *parent, const char* panelName, const char* image = NULL, const char* command = NULL, bool bCreateAuxPanel = true );
		
		// static methods
		static void SetLocalImage( vgui::ImagePanel *control, const char* image );
		static bool CheckImageExists( const char *image );

		// not STATIC
		void SetLocalImage( const char* image );
		void SetLocalAuxImage( const char* image );

		vgui::ImagePanel *GetAuxImagePanel( void ) { return m_pAuxImage; }

		virtual void ApplySchemeSettings( IScheme *pScheme );
		virtual void PerformLayout();
		virtual void OnCursorEntered();
		virtual void OnCursorExited();

		virtual void OnMousePressed(MouseCode code);

		void SetCommandEnabled( bool b ) { m_bCommand = b; }

		virtual void SetMouseOverAction( const char* action ) { Q_strcpy( m_szMouseOverCommand, action ); }
		virtual void SetMouseOutAction( const char* action ) { Q_strcpy( m_szMouseOutCommand, action ); }

	private:
		char m_szCommand[64];
		char m_szMouseOverCommand[64];
		char m_szMouseOutCommand[64];

		bool m_bCommand;

		vgui::ImagePanel *m_pAuxImage;

		CPanelAnimationVar(Color, m_DimmedAlpha, "DimmedColor", "255 255 255 255" );
		CPanelAnimationVar(Color, m_LitAlpha, "LitColor", "255 255 255 255" );
		CPanelAnimationVar(Color, m_AuxDimAlpha, "AuxDimColor", "255 255 255 175" );
		CPanelAnimationVar(Color, m_AuxLitAlpha, "AuxLitColor", "255 255 255 255" );

		CPanelAnimationVarAliasType( int, m_iAuxImagePosX, "AuxImageX", "0", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iAuxImagePosY, "AuxImageY", "0", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iAuxImageWide, "AuxImageW", "0", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iAuxImageHigh, "AuxImageH", "0", "proportional_int" );
	};
}
#endif