//========= Copyright © 2007, Neil Jedrzejewski. ============//
//
// Purpose: VGUI Model Panel element
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "iefx.h"
#include "dlight.h"
#include "view_shared.h"
#include "view.h"
#include "model_types.h"
#include "haj_modelpanel.h"
#include "datacache/imdlcache.h"
#include <KeyValues.h>
#include "c_hl2mp_player.h"
#include "hl2mp_player_shared.h"

CUtlVector<CModelPanel*> g_ModelPanels;	// list of Model panels

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModelPanel::CModelPanel( vgui::Panel *pParent, const char *pName ) : vgui::ImagePanel( pParent, pName )
{
    g_ModelPanels.AddToTail( this );
    m_ModelName[0] = 0;
	m_SequenceName[0] = 0;
	m_WeaponModel[0] = 0;
	m_WeaponSequence[0] = 0;
	m_fov = 54;
	m_Origin = Vector(0,0,0);

	SetParent( pParent );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CModelPanel::~CModelPanel()
{
    g_ModelPanels.FindAndRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModelPanel::ApplySettings( KeyValues *inResourceData )
{
    // main model
	const char *pName = inResourceData->GetString( "3dmodel" );
    if ( pName )
		Q_strncpy( m_ModelName, pName, sizeof( m_ModelName ) );

	// main model sequence
	const char *pSeq = inResourceData->GetString( "sequence" );
    if ( pSeq )
        Q_strncpy( m_SequenceName, pSeq, sizeof( m_SequenceName ) );

	// weapon model
	const char *pWep = inResourceData->GetString( "weaponmodel" );
    if ( pWep )
        Q_strncpy( m_WeaponModel, pWep, sizeof( m_WeaponModel ) );
    
	//weapon model sequence
	const char *pWepSeq = inResourceData->GetString( "weaponsequence" );
    if ( pWepSeq )
        Q_strncpy( m_WeaponSequence, pWepSeq, sizeof( m_WeaponSequence ) );
    
	// fov
	m_fov = inResourceData->GetInt( "fov", 54);
    
	// origin
	m_Origin.x = inResourceData->GetInt( "originx", -110);
	m_Origin.y = inResourceData->GetInt( "originy", -5);
	m_Origin.z = inResourceData->GetInt( "originz", -5);
    
	BaseClass::ApplySettings( inResourceData );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModelPanel::Paint()
{
    BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: See if we should be creating or recreating the model instances
//-----------------------------------------------------------------------------
bool CModelPanel::ShouldRecreateClassImageEntity( C_BaseAnimating *pEnt, const char *pNewModelName )
{
    if ( !pNewModelName || !pNewModelName[0] )
        return false;
    
	if ( !pEnt )
        return true;

    const model_t *pModel = pEnt->GetModel();

    if ( !pModel )
        return true;
    
	const char *pName = modelinfo->GetModelName( pModel );

    if ( !pName )
        return true;

    // reload only if names are different
    return( Q_stricmp( pName, pNewModelName ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Determines if the VGUI panel is visible
// Notes: This seems awful clumsy...
//-----------------------------------------------------------------------------
bool CModelPanel::WillPanelBeVisible( vgui::VPANEL hPanel )
{
    while ( hPanel )
    {
        if ( !vgui::ipanel()->IsVisible( hPanel ) )
            return false;
        hPanel = vgui::ipanel()->GetParent( hPanel );
    }
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update and animate the model
//-----------------------------------------------------------------------------
void CModelPanel::UpdateClassImageEntity( const char *pModelName, const char *pModelSequence, const char *pWeaponName, const char *pWeaponSequence, int x, int y, int width, int height )
{
	if( !IsVisible() )
		return;

    C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

    if ( !pLocalPlayer )
        return;

	MDLCACHE_CRITICAL_SECTION();

   // C_BaseAnimatingOverlay *pPlayerModel = g_ClassImagePlayer.Get();
	C_BaseFlex *pPlayerModel = g_ClassImagePlayer.Get();
	
    // Does the entity even exist yet?
    bool recreatePlayer = ShouldRecreateClassImageEntity( pPlayerModel, pModelName );

	if ( recreatePlayer )
    {
        // if the pointer already exists, remove it as we create a new one.
        if ( pPlayerModel )
            pPlayerModel->Remove();

        // create a new instance
		//pPlayerModel = new C_BaseAnimatingOverlay;
		pPlayerModel = new C_BaseFlex;
		pPlayerModel->InitializeAsClientEntity( pModelName, RENDER_GROUP_OPAQUE_ENTITY );
		pPlayerModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally
		pPlayerModel->AddEffects( EF_NOINTERP );

		// have the player stand in the menuidle pose.
		// Maybe have this as a parameter?
        pPlayerModel->SetSequence( pPlayerModel->LookupSequence( pModelSequence ) );
		pPlayerModel->SetPoseParameter( "move_yaw", 90.0f );	// move_yaw
        pPlayerModel->SetPoseParameter( "head_pitch", 10.0f );	// body_pitch, look down a bit
        //pPlayerModel->SetPoseParameter( 2, 0.0f );	// body_yaw
        //pPlayerModel->SetPoseParameter( 3, 0.0f );	// move_y
        //pPlayerModel->SetPoseParameter( 4, 0.0f );	// move_x

		// show a helmet
		pPlayerModel->SetBodygroup( 2, 1 );
        
		g_ClassImagePlayer = pPlayerModel;
    }

	if( pPlayerModel )
	{
		Vector vForward;
		AngleVectors( pPlayerModel->GetLocalAngles(), &vForward );

		pPlayerModel->m_viewtarget = pPlayerModel->GetAbsOrigin() + vForward * 512;
	}

	C_BaseAnimating *pWeaponModel = g_ClassImageWeapon.Get();

    // Does the entity even exist yet?
    if ( recreatePlayer ) //|| ShouldRecreateClassImageEntity( pWeaponModel, pWeaponName ) )
    {
        if ( pWeaponModel )
            pWeaponModel->Remove();

        pWeaponModel = new C_BaseAnimating;
        pWeaponModel->InitializeAsClientEntity( pWeaponName, RENDER_GROUP_OPAQUE_ENTITY );
        pWeaponModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally
        pWeaponModel->FollowEntity( pPlayerModel ); // attach to player model
		pWeaponModel->SetSequence( pWeaponModel->LookupSequence( pWeaponSequence ) );
        g_ClassImageWeapon = pWeaponModel;
    }

	Vector origin = pLocalPlayer->EyePosition();
    Vector lightOrigin = origin;

    // find a spot inside the world for the dlight's origin, or it won't illuminate the model
	trace_t tr;

	Vector testPos( origin.x - 100, origin.y, origin.z + 100 );
    UTIL_TraceLine( origin, testPos, MASK_OPAQUE, pLocalPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction == 1.0f )
    {
        lightOrigin = tr.endpos;
    }
    else
    {
        // Now move the model away so we get the correct illumination
        lightOrigin = tr.endpos + Vector( 1, 0, -1 );	// pull out from the solid
        Vector start = lightOrigin;
        Vector end = lightOrigin + Vector( 100, 0, -100 );
        UTIL_TraceLine( start, end, MASK_OPAQUE, pLocalPlayer, COLLISION_GROUP_NONE, &tr );
        //origin = tr.endpos;
    }

    float ambient = engine->GetLightForPoint( origin, true ).Length();

	// Now draw it.
	CViewSetup view;
	// setup the views location, size and fov (amongst others)
	view.x = x;
	view.y = y;
	view.width = width;
	view.height = height;

	view.m_bOrtho = false;
	view.angles.Init();

	// make sure that we see all of the player model
	Vector vMins, vMaxs;
	pPlayerModel->C_BaseAnimating::GetRenderBounds( vMins, vMaxs );

	// This'll need tweaking probably
	// Might be fun here to move the camera up and the view angle down so your
	// looking down on the model?
	view.fov = 28;										// crappy FOV, lets us fill the frame without bad perspective
	view.origin = origin + Vector (-110, -5, -5);		// Camera Origin
	view.origin.z += ( vMins.z + vMaxs.z ) * 0.55f;		// Vertical Position (about half the height of the model)

	view.m_vUnreflectedOrigin = view.origin;
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	// render it out to the new CViewSetup area
	Frustum dummyFrustum;
	render->Push3DView(view, 0, false, NULL, dummyFrustum); 

    // Make a light so the model is well lit.
    // use a non-zero number so we cannibalize ourselves next frame
    dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + pLocalPlayer->entindex() );

	dl->flags = DLIGHT_NO_WORLD_ILLUMINATION;
    dl->origin = lightOrigin;
    // Go away immediately so it doesn't light the world too.
    dl->die = gpGlobals->curtime + 1.0;

    dl->color.r = dl->color.g = dl->color.b = 250;
    if ( ambient < 1.0f )
        dl->color.exponent = 1 + (1 - ambient) * 2;

	dl->radius	= 400;

	// move player model in front of our view
    pPlayerModel->SetAbsOrigin( origin );
    pPlayerModel->SetAbsAngles( QAngle( 0, 210, 0 ) );

    pPlayerModel->FrameAdvance( gpGlobals->frametime );

	pPlayerModel->DrawModel( STUDIO_RENDER );
    if ( pWeaponModel )
       pWeaponModel->DrawModel( STUDIO_RENDER );

	render->PopView(dummyFrustum);


}