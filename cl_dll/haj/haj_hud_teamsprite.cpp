#include "cbase.h"
#include "view.h"
#include "haj_cvars.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"
#include "haj_player_shared.h"
#include "haj_mapsettings_enums.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// *HAJ 020* - Jed
// Player team sprite lists

// British
const char *g_ppszBritishSprites[] = 
{
	"hud/oh_uk_icon",
	"hud/oh_uk_icon_lcpl",
	"hud/oh_uk_icon_cpl",
	"hud/oh_uk_icon_sgt",
	"hud/oh_uk_icon_ssgt",
};

// Canadian
const char *g_ppszCanadianSprites[] = 
{
	"hud/oh_uk_icon",
	"hud/oh_uk_icon_lcpl",
	"hud/oh_uk_icon_cpl",
	"hud/oh_uk_icon_sgt",
	"hud/oh_uk_icon_ssgt",
};

// Polish
const char *g_ppszPolishSprites[] = 
{
	"hud/oh_pol_icon",
	"hud/oh_pol_icon_lcpl",
	"hud/oh_pol_icon_cpl",
	"hud/oh_pol_icon_sgt",
	"hud/oh_pol_icon_ssgt",
};

// German
const char *g_ppszGermanSprites[] = 
{
	"hud/oh_ger_icon",
	"hud/oh_ger_icon_lcpl",
	"hud/oh_ger_icon_cpl",
	"hud/oh_ger_icon_sgt",
	"hud/oh_ger_icon_ssgt",
};

// Cvars for size and height
ConVar haj_cl_teamsprites_height( "haj_cl_teamsprites_height", "43", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Z offset for overhead team sprites"); 
ConVar haj_cl_teamsprites_size( "haj_cl_teamsprites_size", "8", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Size of overhead team sprites");
ConVar haj_cl_teamsprites_drawlocalplayer( "haj_cl_teamsprites_drawlocalplayer", "0", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Draw local player's icon? Useful for testing." );

//-----------------------------------------------------------------------------
// Purpose: Draw team/rank sprite over head.
//-----------------------------------------------------------------------------
void C_HajPlayer::DrawTeamSprite()
{
	if( IsLocalPlayer() && !haj_cl_teamsprites_drawlocalplayer.GetBool() ) // don't draw local player
		return;

	int pTeam = GetTeamNumber();

	if ( !m_pTeamSprite || pTeam != m_iSpriteTeam || gpGlobals->curtime >= m_flNextSpriteCheck )
	{
		// Try and grab our game rules.
		CHajGameRules *pGameRules = HajGameRules();
		
		if (!pGameRules)
			return;

		char szSprite[MAX_PATH];

		if ( pTeam == TEAM_CWEALTH )
		{
			switch ( pGameRules->GetCommonwealthNation() )
			{
				default:
				case NATION_CWEALTH_BRITAIN:
					Q_strcpy( szSprite, g_ppszBritishSprites[GetCurrentRank()] );
					break;
			
				case NATION_CWEALTH_CANADA:
					Q_strcpy( szSprite, g_ppszCanadianSprites[GetCurrentRank()] );
					break;
				
				case NATION_CWEALTH_POLAND:
					Q_strcpy( szSprite, g_ppszPolishSprites[GetCurrentRank()] );
					break;
			}
		}

		// Axis
		else
		{
			switch ( pGameRules->GetAxisNation() )
			{
				default:
				case NATION_AXIS_GERMANY:
					Q_strcpy( szSprite, g_ppszGermanSprites[GetCurrentRank()] );
					break;
			}
		}

		m_pTeamSprite = materials->FindMaterial( szSprite, TEXTURE_GROUP_VGUI );
		m_iSpriteTeam = pTeam;
		m_flNextSpriteCheck = gpGlobals->curtime + 5.0f;
	}

	// Place it 20 units above his head.
	Vector vOrigin = WorldSpaceCenter();
	vOrigin.z += haj_cl_teamsprites_height.GetFloat();

		
	// Align it so it never points up or down.
	Vector vUp( 0, 0, 1 );
	Vector vRight = CurrentViewRight();
	if ( fabs( vRight.z ) > 0.95 )	// don't draw it edge-on
		return;

	vRight.z = 0;
	VectorNormalize( vRight );

	float flSize = haj_cl_teamsprites_size.GetFloat();

	materials->Bind( m_pTeamSprite );

	IMesh *pMesh = materials->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Color3f( 1.0, 1.0, 1.0 );
	meshBuilder.TexCoord2f( 0,0,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( 1.0, 1.0, 1.0 );
	meshBuilder.TexCoord2f( 0,1,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( 1.0, 1.0, 1.0 );
	meshBuilder.TexCoord2f( 0,1,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( 1.0, 1.0, 1.0 );
	meshBuilder.TexCoord2f( 0,0,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();
	meshBuilder.End();

	pMesh->Draw();
}