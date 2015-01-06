//========= Copyright © 2009 Stephen Swires <stephen.swires@gmail.com> ============//
//
// Purpose: Dynamic crosshairs (like CS, L4D, COD etc)
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "iinput.h"
#include "input.h"
#include "haj_player_c.h"
#include "view.h"

#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "haj_weapon_base.h"

extern ConVar crosshair;

// additional convars
ConVar crosshair_dynamic( "crosshair_dynamic", "0", FCVAR_ARCHIVE );
ConVar crosshair_r( "crosshair_r", "255", FCVAR_ARCHIVE );
ConVar crosshair_g( "crosshair_g", "255", FCVAR_ARCHIVE );
ConVar crosshair_b( "crosshair_b", "255", FCVAR_ARCHIVE );
ConVar crosshair_a( "crosshair_a", "200", FCVAR_ARCHIVE );
ConVar crosshair_border( "crosshair_border", "1", FCVAR_ARCHIVE );
ConVar crosshair_border_r( "crosshair_border_r", "0", FCVAR_ARCHIVE );
ConVar crosshair_border_g( "crosshair_border_g", "0", FCVAR_ARCHIVE );
ConVar crosshair_border_b( "crosshair_border_b", "0", FCVAR_ARCHIVE );
ConVar crosshair_scale( "crosshair_scale", "1", FCVAR_ARCHIVE );
ConVar crosshair_gap_modifier( "crosshair_gap_modifier", "250", FCVAR_CHEAT );
ConVar crosshair_thickness_scale( "crosshair_thickness_scale", "2", FCVAR_ARCHIVE );


class CHudCrosshairDynamic : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCrosshairDynamic, vgui::Panel );

public:
	CHudCrosshairDynamic( const char *pElementName );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void Paint();
	virtual void Think();
	virtual bool ShouldDraw();

	void GetCrosshairOrigin( int &x, int &y );

	virtual void PaintCrosshairBit( int x, int y, int w, int h );

	float m_flCurrentGap; // used to draw the element, used to approach new value
};

DECLARE_HUDELEMENT( CHudCrosshairDynamic );

CHudCrosshairDynamic::CHudCrosshairDynamic( const char *pElementName ) :
CHudElement( pElementName ), BaseClass( NULL, "HudCrosshairDynamic" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetProportional( false );
	SetPos( 0, 0 );
	SetSize( ScreenWidth(), ScreenHeight() );

	m_flCurrentGap = 0.0f;
}

void CHudCrosshairDynamic::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
	SetPos( 0, 0 );
	SetSize( ScreenWidth(), ScreenHeight() );
}

// CHudCrosshairDynamic::ShouldDraw
// Purpose: Should we draw the crosshair?
extern ConVar vm_ironsight_adjust;
bool CHudCrosshairDynamic::ShouldDraw()
{
	if( vm_ironsight_adjust.GetBool() )
		return true;

	if( !g_pClientMode->ShouldDrawCrosshair() || !crosshair_dynamic.GetBool() )
		return false;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( pPlayer && pPlayer->entindex() == render->GetViewEntity() && ( pPlayer->IsAlive() || ( pPlayer->IsObserver() && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) ) )
	{
		CHAJWeaponBase *pWeapon = dynamic_cast< CHAJWeaponBase* >( pPlayer->GetActiveWeapon() );
		if ( !pWeapon || ( (pWeapon->m_bIsBipodDeployed || ( pWeapon->IsIronsighted() && gpGlobals->curtime >= pWeapon->m_flIronsightTime)) && !input->CAM_IsThirdPerson() ) || pWeapon->ShouldForceTextureCrosshair() /*&& !pWeapon->ShouldDrawCrosshair()*/ )
			return false;

		return true;
	}

	return false;
}

// CHudCrosshairDynamic::Think
// Purpose: Panel's think function, here we calculate the gap we use when drawing the crosshair
void CHudCrosshairDynamic::Think()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( pPlayer )
	{
		CHAJWeaponBase *pWeapon = dynamic_cast< CHAJWeaponBase* >( pPlayer->GetActiveWeapon() );

		if( pWeapon )
		{
			float flWeaponSpread = ( pWeapon->GetBulletSpread().Length2D() * crosshair_gap_modifier.GetFloat() );
			m_flCurrentGap = Approach( flWeaponSpread, m_flCurrentGap, gpGlobals->frametime * 100 );

			return;
		}
	}

	m_flCurrentGap = 0.0f; // null player

}

// CHudCrosshairDynamic::PaintCrosshairBit( int, int, int, int )
// Purpose: paints a bit of the crosshair on the screen, calcs border etc
void CHudCrosshairDynamic::PaintCrosshairBit( int x, int y, int w, int h )
{
	int border = crosshair_border.GetInt();
	int alpha = crosshair_a.GetInt();

	if( m_flCurrentGap > 30.0f )
		alpha += -( ( m_flCurrentGap - 30 ) * 5 );

	if( border > 0 && w > 2 && h > 2 ) // paint border first
	{
		vgui::surface()->DrawSetColor( Color( crosshair_border_r.GetInt(), crosshair_border_g.GetInt(), crosshair_border_b.GetInt(), clamp( alpha, 0, 255 ) ) )	;
		vgui::surface()->DrawFilledRect( x, y, x + w, y + border ); // top
		vgui::surface()->DrawFilledRect( x, y + border, x + border, y + h - ( border / 2 ) ); // left
		vgui::surface()->DrawFilledRect( x + w - border, y + border, x + w, y + h - ( border / 2 ) ); // right
		vgui::surface()->DrawFilledRect( x + border, y + h - border, x + w - border, y + h ); // bottom
	}
	else
		border = 0;

	if( w < 1 )
		w = 1;

	if( h < 1 )
		h = 1;

	vgui::surface()->DrawSetColor( Color( crosshair_r.GetInt(), crosshair_g.GetInt(), crosshair_b.GetInt(), clamp( alpha, 0, 255 ) ) );
	vgui::surface()->DrawFilledRect( x + border, y + border, x + w - border, y + h - border ); // middle

}

// CHudCrosshairDynamic::Paint
// Purpose: takes the calculate gap and draws the crosshair on screen
void CHudCrosshairDynamic::Paint( )
{
	float scale = ( crosshair_scale.GetFloat() * 1.5 ) * ( ScreenHeight() / 720 );

	int gap = (int)m_flCurrentGap * scale;
	int length = ( gap + ( 13 * scale ) ) * 0.6;
	int thickness = ceil( crosshair_thickness_scale.GetFloat() * scale );

	int cx = 0;
	int cy = 0;

	GetCrosshairOrigin( cx, cy );

	PaintCrosshairBit( cx - gap - length, cy-ceil( (float)(thickness/2) ), length, thickness ); // left
	PaintCrosshairBit( cx + gap + ceil( (float)(thickness/2) ), cy-floor( (float)(thickness/2) ), length, thickness ); // right
	PaintCrosshairBit( cx - floor( (float)(thickness/2) ), cy - gap - length, thickness, length ); // top
	PaintCrosshairBit( cx - floor( (float)(thickness/2) ), cy + gap + ceil( (float)(thickness/2) ), thickness, length ); // bottom
}

void CHudCrosshairDynamic::GetCrosshairOrigin( int &x, int &y )
{
	if( input->CAM_IsThirdPerson() )
	{
		C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

		if( pPlayer )
		{
			trace_t tr;
			Vector forward;
			pPlayer->EyeVectors( &forward );

			UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
			GetVectorInScreenSpace( tr.endpos, x, y );
		}
	}
	else
	{
		x = ScreenWidth() / 2;
		y = ScreenHeight() / 2;
	}
}