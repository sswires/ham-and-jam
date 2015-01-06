#ifndef HAJ_MODELPANEL_H
#define HAJ_MODELPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ImagePanel.h>
#include <cl_dll/iviewport.h>
#include <UtlVector.h>
#include "c_hl2mp_player.h"
#include "hl2mp_player_shared.h"

namespace vgui
{
	class ImagePanel;
}

class CModelPanel : public vgui::ImagePanel
{
    public:

        typedef vgui::ImagePanel BaseClass;

        CModelPanel( vgui::Panel *pParent, const char *pName );
        virtual ~CModelPanel();
        virtual void ApplySettings( KeyValues *inResourceData );
        virtual void Paint();


    public:
        char m_ModelName[128];
		char m_SequenceName[128];
		char m_WeaponModel[128];
		char m_WeaponSequence[128];
		int m_fov;
		Vector m_Origin;

		//CHandle<C_BaseAnimatingOverlay> g_ClassImagePlayer;		// player
		//CHandle<C_BaseAnimating> g_ClassImageWeapon;			// weapon

		CHandle<C_BaseFlex> g_ClassImagePlayer;		// player
		CHandle<C_BaseAnimating> g_ClassImageWeapon;			// weapon


		bool ShouldRecreateClassImageEntity( C_BaseAnimating *pEnt, const char *pNewModelName );
		bool WillPanelBeVisible( vgui::VPANEL hPanel );
		void UpdateClassImageEntity( const char *pModelName, const char *pModelSequence, const char *pWeaponName, const char *pWeaponSequence, int x, int y, int width, int height );
};

extern CUtlVector<CModelPanel*> g_ModelPanels;

#endif // HAJ_MODELPANEL_H