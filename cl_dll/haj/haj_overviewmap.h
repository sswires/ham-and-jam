//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ Map Overview Class
// Notes:    This replaces the default CMapOverview class so we can 
//			 customise it without touching the base.
// $NoKeywords: $
//=======================================================================//

#ifndef HAJOVERVIEW_H
#define HAJOVERVIEW_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <cl_dll/iviewport.h>
#include <vector.h>
#include <igameevents.h>
#include <shareddefs.h>
#include <const.h>
#include "hudelement.h"
#include "mapoverview.h"
#include "haj_objectivemanager_c.h"
#include "haj_capturepoint_c.h"

// our own overviewmap class so we dont need to futz with the default one.
class CHAJMapOverview : public CMapOverview
{
	DECLARE_CLASS_SIMPLE( CHAJMapOverview, CMapOverview );

public:
	CHAJMapOverview( const char *pElementName );
	~CHAJMapOverview();

public:
	virtual const char	*GetName( void ) { return PANEL_OVERVIEW; }
	virtual bool		CanPlayerBeSeen( MapPlayer_t *player );
	virtual bool		CanPlayerHealthBeSeen(MapPlayer_t *player);

	virtual void		SetMode( int mode );
	void				DrawObjectives( void );

	virtual void		Paint( void );
	virtual bool		ShouldDraw( void );
	virtual void		OnThink();
	virtual	void		FireGameEvent( IGameEvent *event );

	virtual void		InitTeamColorsAndIcons();

protected:
	int					FindOrCreateTexID( char* texname );
	int					GetFlagTextureId( C_HajObjective* pObjective, int teamId );

private:
	int					m_iLockedIconID;	// "locked" icon texture ID
};

#endif