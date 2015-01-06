//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Hud element that indicates the direction of damage taken by the player
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "text_message.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "vguimatsurface/IMatSystemSurface.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/imaterialvar.h"
#include "ieffects.h"
#include "hudelement.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"
#include "idebugoverlaypanel.h"
#include "engine/IVDebugOverlay.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudDamageDirection : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDamageDirection, vgui::Panel );
public:
	CHudDamageDirection( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset( void );
	virtual bool ShouldDraw( void );

	// Handler for our message
	void MsgFunc_Damage( bf_read &msg );

private:
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	const Vector& GetPlayerOrigin();

	// Painting
	void GetDamagePosition( const Vector &vecDamagePos, float flRadius, float *xpos, float *ypos, float *flRotation );
	void DrawDamageDirection(int iMaterial, int x0, int y0, int x1, int y1, float alpha, float flRotation );

private:
	// Indication times

	CPanelAnimationVarAliasType( float, m_flMinimumWidth, "MinimumWidth", "256", "proportional_float" ); // 800
	CPanelAnimationVarAliasType( float, m_flMaximumWidth, "MaximumWidth", "256", "proportional_float" ); // 800
	CPanelAnimationVarAliasType( float, m_flMinimumHeight, "MinimumHeight", "128", "proportional_float" ); // 400
	CPanelAnimationVarAliasType( float, m_flMaximumHeight, "MaximumHeight", "128", "proportional_float" ); // 400
	CPanelAnimationVarAliasType( float, m_flStartRadius, "StartRadius", "220", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flEndRadius, "EndRadius", "200", "proportional_float" );

	CPanelAnimationVar( float, m_iMaximumDamage, "MaximumDamage", "100" );
	CPanelAnimationVar( float, m_flMinimumTime, "MinimumTime", "1" );
	CPanelAnimationVar( float, m_flMaximumTime, "MaximumTime", "2" );
	CPanelAnimationVar( float, m_flTravelTime, "TravelTime", ".1" );
	CPanelAnimationVar( float, m_flFadeOutPercentage, "FadeOutPercentage", "0.7" );
	CPanelAnimationVar( float, m_flNoise, "Noise", "0.02" );

	// List of damages we've taken
	struct damage_t
	{
		int		iScale;
		float	flLifeTime;
		float	flStartTime;
		Vector	vecOrigin;
		int		material;
		int		damagetype;
		int		iEntIndex;
	};
	CUtlVector<damage_t>	m_vecDamages;

	CMaterialReference m_WhiteAdditiveMaterial[2];
};

ConVar haj_damageindicator_hurt_flash_alpha("haj_cl_damageindicator_hurt_flash_alpha", "0.12", FCVAR_NONE, "The alpha of the screen flash when the player is at low health (0.0 - 1.0)");
ConVar haj_damageindicator_scale("haj_cl_damageindicator_scale", "0.5", FCVAR_ARCHIVE, "Scales the HUD damage indicator");
ConVar haj_damageindicator_alpha("haj_cl_damageindicator_alpha", "1", FCVAR_ARCHIVE, "Alpha of the HUD damage indicator");

ConVar haj_cl_dmgindicator_time( "haj_cl_damageindicator_time", "3.0", FCVAR_ARCHIVE );


DECLARE_HUDELEMENT( CHudDamageDirection );
DECLARE_HUD_MESSAGE( CHudDamageDirection, Damage );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudDamageDirection::CHudDamageDirection( const char *pElementName ) :
CHudElement( pElementName ), BaseClass(NULL, "HudDamageDirection")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_WhiteAdditiveMaterial[0].Init( "hud/hud_hit", TEXTURE_GROUP_VGUI ); 
	m_WhiteAdditiveMaterial[1].Init( "hud/hud_hit", TEXTURE_GROUP_VGUI ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageDirection::Init( void )
{
	HOOK_HUD_MESSAGE( CHudDamageDirection, Damage );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageDirection::Reset( void )
{
	m_vecDamages.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageDirection::VidInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageDirection::OnThink()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudDamageDirection::ShouldDraw( void )
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	// Don't draw if we don't have any damage to indicate
	if ( !m_vecDamages.Count() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Convert a damage position in world units to the screen's units
//-----------------------------------------------------------------------------
void CHudDamageDirection::GetDamagePosition( const Vector &vecDamagePos, float flRadius, float *xpos, float *ypos, float *flRotation )
{
	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();
	Vector forward, right, up;

	AngleVectors(playerAngles, &forward, &right, &up ); // forward normalised vector
	Vector vecDamageNormal = ( vecDamagePos-playerPosition ); // difference between player pos and where we've taken damage
	VectorNormalize( vecDamageNormal );

	Vector vecLocalDir;
	vecLocalDir.x = DotProduct( vecDamageNormal, -right ) * flRadius;
	vecLocalDir.y = DotProduct( vecDamageNormal, forward ) * flRadius;

	*flRotation = atan2( vecLocalDir.x, vecLocalDir.y );
	*flRotation *= 180 / M_PI;

	float yawRadians = -(*flRotation) * M_PI / 180.0f;
	float ca = cos( yawRadians );
	float sa = sin( yawRadians );

	// Rotate it around the circle
	*xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
	*ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: Draw a single damage indicator
//-----------------------------------------------------------------------------
void CHudDamageDirection::DrawDamageDirection(int iMaterial, int x0, int y0, int x1, int y1, float alpha, float flRotation )
{
	//CMatRenderContextPtr pRenderContext( materials );
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, m_WhiteAdditiveMaterial[1] );

	// Get the corners, since they're being rotated
	int wide = x1 - x0;
	int tall = y1 - y0;
	Vector2D vecCorners[4];
	Vector2D center( x0 + (wide * 0.5f), y0 + (tall * 0.5f) );
	float yawRadians = -flRotation * M_PI / 180.0f;
	Vector2D axis[2];
	axis[0].x = cos(yawRadians);
	axis[0].y = sin(yawRadians);
	axis[1].x = -axis[0].y;
	axis[1].y = axis[0].x;
	Vector2DMA( center, -0.5f * wide, axis[0], vecCorners[0] );
	Vector2DMA( vecCorners[0], -0.5f * tall, axis[1], vecCorners[0] );
	Vector2DMA( vecCorners[0], wide, axis[0], vecCorners[1] );
	Vector2DMA( vecCorners[1], tall, axis[1], vecCorners[2] );
	Vector2DMA( vecCorners[0], tall, axis[1], vecCorners[3] );

	// Draw the sucker
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	int iAlpha = alpha * 255;
	meshBuilder.Color4ub( 255,255,255, iAlpha );
	meshBuilder.TexCoord2f( 0,0,0 );
	meshBuilder.Position3f( vecCorners[0].x,vecCorners[0].y,0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( 255,255,255, iAlpha );
	meshBuilder.TexCoord2f( 0,1,0 );
	meshBuilder.Position3f( vecCorners[1].x,vecCorners[1].y, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( 255,255,255, iAlpha );
	meshBuilder.TexCoord2f( 0,1,1 );
	meshBuilder.Position3f( vecCorners[2].x,vecCorners[2].y, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( 255,255,255, iAlpha );
	meshBuilder.TexCoord2f( 0,0,1 );
	meshBuilder.Position3f( vecCorners[3].x,vecCorners[3].y, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageDirection::Paint()
{
	// Iterate backwards, because we might remove them as we go
	int iSize = m_vecDamages.Count();
	for (int i = iSize-1; i >= 0; i--)
	{
		// Scale size to the damage
		int clampedDamage = clamp( m_vecDamages[i].iScale, 0, m_iMaximumDamage );

		float fMinWidth = m_flMinimumWidth * haj_damageindicator_scale.GetFloat();
		float fMaxWidth = m_flMaximumWidth * haj_damageindicator_scale.GetFloat();
		float fMinHeight = m_flMinimumHeight * haj_damageindicator_scale.GetFloat();
		float fMaxHeight = m_flMaximumHeight * haj_damageindicator_scale.GetFloat();

		int iWidth = RemapVal(clampedDamage, 0, m_iMaximumDamage, fMinWidth, fMaxWidth ) * 0.5;
		int iHeight = RemapVal(clampedDamage, 0, m_iMaximumDamage, fMinHeight, fMaxHeight ) * 0.5;

		// Find the place to draw it
		float xpos, ypos;
		float flRotation;
		float flTimeSinceStart = ( gpGlobals->curtime - m_vecDamages[i].flStartTime );
		float flRadius = RemapVal( min( flTimeSinceStart, m_flTravelTime ), 0, m_flTravelTime, m_flStartRadius, m_flEndRadius );

		flRadius *= haj_damageindicator_scale.GetFloat();

		GetDamagePosition( m_vecDamages[i].vecOrigin, flRadius, &xpos, &ypos, &flRotation );

		float flAnimWidth = iWidth;

		// Calculate life left
		float flLifeLeft = ( m_vecDamages[i].flLifeTime - gpGlobals->curtime );
		if ( flLifeLeft > 0 )
		{
			float flPercent = flTimeSinceStart / (m_vecDamages[i].flLifeTime - m_vecDamages[i].flStartTime);
			float alpha;
			if ( flPercent <= m_flFadeOutPercentage )
			{
				alpha = 1.0;
			}
			else
			{
				alpha = 1.0 - RemapVal( flPercent, m_flFadeOutPercentage, 1.0, 0.0, 1.0 );
				//flAnimWidth = iWidth * alpha;
			}

			alpha *= haj_damageindicator_alpha.GetFloat();
			DrawDamageDirection( m_vecDamages[i].material, xpos-flAnimWidth, ypos-iHeight, xpos+flAnimWidth, ypos+iHeight, alpha, flRotation );
		}
		else
		{
			m_vecDamages.Remove(i);
		}
	}
}

const Vector& CHudDamageDirection::GetPlayerOrigin()
{
	return MainViewOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageDirection::MsgFunc_Damage( bf_read &msg )
{
/*
WRITE_BYTE( m_DmgSave );
WRITE_BYTE( m_DmgTake );
WRITE_LONG( visibleDamageBits );
WRITE_FLOAT( damageOrigin.x );	//BUG: Should be fixed point (to hud) not floats
WRITE_FLOAT( damageOrigin.y );	//BUG: However, the HUD does _not_ implement bitfield messages (yet)
WRITE_FLOAT( damageOrigin.z );	//BUG: We use WRITE_VEC3COORD for everything else
*/

	int iDmgSave = msg.ReadByte();
	int iDmgTake = msg.ReadByte();
	int iDamageType = msg.ReadLong();
	Vector vecSrc = Vector( msg.ReadFloat(), msg.ReadFloat(), msg.ReadFloat() );

	if( iDmgTake > 0 )
	{
		damage_t damage;
		damage.iScale = iDmgTake;
		damage.damagetype = iDamageType;

		if ( damage.iScale > m_iMaximumDamage )
		{
			damage.iScale = m_iMaximumDamage;
		}

		damage.vecOrigin = vecSrc;
		damage.flStartTime = gpGlobals->curtime;
		damage.flLifeTime = gpGlobals->curtime + haj_cl_dmgindicator_time.GetFloat();

		if ( damage.vecOrigin == vec3_origin )
		{
			damage.vecOrigin = GetPlayerOrigin();
		}

		// add a slash for drones
		damage.material = 1;
		m_vecDamages.AddToTail( damage );
	}

}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudDamageDirection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	// set our size
	int screenWide, screenTall;
	int x, y;
	GetPos(x, y);
	GetHudSize(screenWide, screenTall);
	SetBounds(0, y, screenWide, screenTall - y);
}