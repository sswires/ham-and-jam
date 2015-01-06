//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: The Ham and Jam concussion effect.
// Note:	This can be called via C_HaJScreenEffects or env_screenoverlay
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_CONCUSSIONEFFECT_H
#define HAJ_CONCUSSIONEFFECT_H
#ifdef _WIN32
#pragma once
#endif

#include "screenspaceeffects.h"

class CConcussionEffect : public IScreenSpaceEffect
{
public:
	CConcussionEffect( void ) : 
		m_pStunTexture( NULL ), 
		m_flDuration( 0.0f ), 
		m_flFinishTime( 0.0f ), 
		m_bUpdated( false ) {}

	virtual void Init( void );
	virtual void Shutdown( void ) {}
	virtual void SetParameters( KeyValues *params );
	virtual void Enable( bool bEnable ) {};
	virtual bool IsEnabled( ) { return true; }

	virtual void Render( int x, int y, int w, int h );

private:
	ITexture	*m_pStunTexture;
	float		m_flDuration;
	float		m_flFinishTime;
	bool		m_bUpdated;


	IMaterial	*screen_mat;
	IMaterial	*downsample_mat;
	IMaterial	*blurx_mat;
	IMaterial	*blury_mat;

	ITexture	*dest_rt0;
	ITexture	*dest_rt1;

	IMaterialVar *mv;

};

ADD_SCREENSPACE_EFFECT( CConcussionEffect, haj_concussion );

#endif // HAJ_CONCUSSIONEFFECT_H