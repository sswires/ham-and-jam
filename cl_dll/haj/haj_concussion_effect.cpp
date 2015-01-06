//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: The Ham and Jam concussion effect.
// Note:	This can be called via C_HaJScreenEffects or env_screenoverlay
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "screenspaceeffects.h"
#include "rendertexture.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "cdll_client_int.h"
#include "materialsystem/itexture.h"
#include "keyvalues.h"
#include "ClientEffectPrecacheSystem.h"

#include "haj_concussion_effect.h"

const float MAX_CONCUSS_DURATION = 8.0f;	// duration of concussion for 100 damage (seconds)

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectHaJConcussion )
//CLIENTEFFECT_MATERIAL( "effects/stun" )
//CLIENTEFFECT_MATERIAL( "dev/downsample" )
CLIENTEFFECT_MATERIAL( "fx/out" )
CLIENTEFFECT_MATERIAL( "dev/downsample_non_hdr" )
CLIENTEFFECT_MATERIAL( "dev/blurfilterx" )
//CLIENTEFFECT_MATERIAL( "dev/blurfilterx_nohdr" )
CLIENTEFFECT_MATERIAL( "dev/blurfiltery" )
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CConcussionEffect::Init( void ) 
{
	m_pStunTexture = NULL;
	m_flDuration = 0.0f;
	m_flFinishTime = 0.0f;
	m_bUpdated = false;

	screen_mat = NULL;
	downsample_mat = NULL;
	blurx_mat = NULL;
	blury_mat = NULL;

	dest_rt0 = NULL;
	dest_rt1 = NULL;

	mv = NULL;
}

//------------------------------------------------------------------------------
// Purpose: Pick up changes in our parameters
//------------------------------------------------------------------------------
void CConcussionEffect::SetParameters( KeyValues *params )
{
	if( params->FindKey( "duration" ) )
	{
		m_flDuration =  ( MAX_CONCUSS_DURATION / 100.0f ) * params->GetFloat( "duration" );
		m_flFinishTime = gpGlobals->curtime + m_flDuration;
		m_bUpdated = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Render the effect
//-----------------------------------------------------------------------------
void CConcussionEffect::Render( int x, int y, int w, int h )
{
	// Make sure we're ready to play this effect
	if ( m_flFinishTime < gpGlobals->curtime )
		return;

	screen_mat = materials->FindMaterial( "fx/out", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( screen_mat ) )
		return;

	downsample_mat = materials->FindMaterial( "dev/downsample_non_hdr", TEXTURE_GROUP_OTHER, true);
	if( IsErrorMaterial( downsample_mat ) )
		return;

	blurx_mat = materials->FindMaterial( "dev/blurfilterx", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( blurx_mat ) )
		return;
	
	blury_mat = materials->FindMaterial( "dev/blurfiltery", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( blury_mat ) )
		return;

	bool bResetBaseFrame = m_bUpdated;
	int	 w1, h1;
	bool found;

	// Set ourselves to the proper rendermode
	materials->MatrixMode( MATERIAL_VIEW );
	materials->PushMatrix();
	materials->LoadIdentity();
	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PushMatrix();
	materials->LoadIdentity();

	// Save off this pass
	Rect_t srcRect;
	srcRect.x = x;
	srcRect.y = y;
	srcRect.width = w;
	srcRect.height = h;

	ITexture *pSaveRenderTarget = materials->GetRenderTarget();
	ITexture *pFBTexture = GetFullFrameFrameBufferTexture( 0 );
	ITexture *dest_rt0 = materials->FindTexture( "_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET );
	ITexture *dest_rt1 = materials->FindTexture( "_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET );

	// First copy the FB off to the offscreen texture
	materials->CopyRenderTargetToTexture( pFBTexture );

	// get height of downsample buffers
	w1 = dest_rt1->GetActualWidth();
	h1 = dest_rt1->GetActualHeight();

	// set out initial alpha to 1.0 (opaque)
	mv = screen_mat->FindVar( "$alpha", &found, false );
	if ( found ) { mv->SetFloatValue( 1.0f ); }

	// Downsample image
	materials->SetRenderTarget( dest_rt0 );
	materials->Viewport( 0, 0, w1, h1 );
	materials->DrawScreenSpaceQuad( downsample_mat );

	// figure out how long is left and round it down to an int
	float timeleft = m_flFinishTime - gpGlobals->curtime;
	int t = (int)timeleft;
	if (t > 3) t = 3;	// set cap at max 3.

	// blur the downsample buffer 3 times
	// max is 3, down to 1 pass at the end.
	for (; t >= 1; t-- )
	{	
		// Blur filter pass 1
		materials->SetRenderTarget( dest_rt1 );
		materials->Viewport( 0, 0, w, h );
		materials->DrawScreenSpaceQuad( blurx_mat );

		// Blur filter pass 2
		materials->SetRenderTarget( dest_rt0 );
		materials->Viewport( 0, 0, w, h );
		materials->DrawScreenSpaceQuad( blury_mat );
	}

	// set the alpha to fade out over the last 3 seconds.
	mv = screen_mat->FindVar( "$alpha", &found, false );
 	if ( found )
		mv->SetFloatValue( timeleft > 3.0f ? 1.0f : timeleft * 0.333333 );

	materials->SetRenderTarget( pSaveRenderTarget );
	materials->Viewport( srcRect.x,srcRect.y, srcRect.width, srcRect.height );
	materials->DrawScreenSpaceQuad( screen_mat );	

	/*
	// Get our current view
	if ( m_pStunTexture == NULL )
	{
		m_pStunTexture = GetPowerOfTwoFrameBufferTexture();
	}

	// Draw the texture if we're using it
	if ( m_pStunTexture != NULL )
	{
		bool foundVar;
		IMaterialVar* pBaseTextureVar = pMaterial->FindVar( "$basetexture", &foundVar, false );

		if ( bResetBaseFrame )
		{
			// Save off this pass
			Rect_t srcRect;
			srcRect.x = x;
			srcRect.y = y;
			srcRect.width = w;
			srcRect.height = h;
			pBaseTextureVar->SetTextureValue( m_pStunTexture );

			materials->CopyRenderTargetToTextureEx( m_pStunTexture, 0, &srcRect, NULL );
			materials->SetFrameBufferCopyTexture( m_pStunTexture );
			m_bUpdated = false;
		}

		byte overlaycolor[4] = { 255, 255, 255, 0 };

		float flEffectPerc = ( m_flFinishTime - gpGlobals->curtime ) / m_flDuration;
		overlaycolor[3] = (byte) (150.0f * flEffectPerc);

		render->ViewDrawFade( overlaycolor, pMaterial );

		float viewOffs = ( flEffectPerc * 32.0f ) * cos( gpGlobals->curtime * 10.0f * cos( gpGlobals->curtime * 2.0f ) );
		float vX = x + viewOffs;
		float vY = y;

		// just do one pass for dxlevel < 80.
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80)
		{
			materials->DrawScreenSpaceRectangle( pMaterial, vX, vY, w, h,
				0, 0, m_pStunTexture->GetActualWidth()-1, m_pStunTexture->GetActualHeight()-1, 
				m_pStunTexture->GetActualWidth(), m_pStunTexture->GetActualHeight() );

			render->ViewDrawFade( overlaycolor, pMaterial );

			materials->DrawScreenSpaceRectangle( pMaterial, x, y, w, h,
				0, 0, m_pStunTexture->GetActualWidth()-1, m_pStunTexture->GetActualHeight()-1, 
				m_pStunTexture->GetActualWidth(), m_pStunTexture->GetActualHeight() );
		}

		// Save off this pass
		Rect_t srcRect;
		srcRect.x = x;
		srcRect.y = y;
		srcRect.width = w;
		srcRect.height = h;
		pBaseTextureVar->SetTextureValue( m_pStunTexture );

		materials->CopyRenderTargetToTextureEx( m_pStunTexture, 0, &srcRect, NULL );
	}
	*/

	// Restore our state
	materials->MatrixMode( MATERIAL_VIEW );
	materials->PopMatrix();
	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PopMatrix();
}