//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ Map Overview Class
// Notes:    This replaces the default CMapOverview class so we can 
//			 customise it without touching the base.
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_overviewmap.h"
#include "mapoverview.h"
#include <vgui/isurface.h>
#include <vgui/ILocalize.h>
#include <filesystem.h>
#include <keyvalues.h>
#include <convar.h>
#include <mathlib.h>
#include <cl_dll/iviewport.h>
#include <igameresources.h>
#include "gamevars_shared.h"
#include "spectatorgui.h"
#include "c_playerresource.h"
#include "view.h"

#include "clientmode.h"
#include <vgui_controls/AnimationController.h>

#include "haj_hudcapturepoints.h"
#include "haj_capturepoint_c.h"
#include "haj_gamerules.h"
#include "haj_misc.h"
#include "haj_objectivemanager_c.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// size of objective icons on the map
ConVar overview_objective_size( "overview_haj_objective_size",  "16.0", FCVAR_ARCHIVE, "Size of the objective icons on the map\n" );
ConVar overview_objective_alpha( "overview_haj_objective_alpha", "200", FCVAR_ARCHIVE, "Alpha of the objective icons on the map", true, 0, true, 255 );
ConVar overview_player_size( "overview_player_size", "86", FCVAR_ARCHIVE, "Icon size");

ConVar cl_drawradar( "cl_haj_radar", "1", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Draw the overview on the HUD?" );
ConVar cl_preferred_radar_mode( "cl_haj_radar_preferred_mode", "3", FCVAR_ARCHIVE, "Preferred radar mode", true, CMapOverview::MAP_MODE_OFF, true, CMapOverview::MAP_MODE_RADAR );
ConVar cl_radar_alpha( "cl_haj_radar_alpha", "255", FCVAR_ARCHIVE, "Alpha to use on radar", true, 0, true, 255 );

// handle our overview mode command
CON_COMMAND( overview_mode, "Sets overview map mode off,small,large: <0|1|2>" )
{
	if ( !g_pMapOverview )
		return;

	int mode;

	if ( engine->Cmd_Argc() < 2 )
	{
		// toggle modes
		mode = g_pMapOverview->GetMode() + 1;

		if ( mode >  CMapOverview::MAP_MODE_RADAR )
			mode = CMapOverview::MAP_MODE_OFF;
	}
	else
	{
		// set specific mode
		mode = Q_atoi(engine->Cmd_Argv( 1 ));
	}

	if( mode != CMapOverview::MAP_MODE_RADAR )
		g_pMapOverview->SetPlayerPreferredMode( mode );

	if( !g_pMapOverview->AllowConCommandsWhileAlive() )
	{
		C_BasePlayer *localPlayer = CBasePlayer::GetLocalPlayer();
		if( localPlayer && CBasePlayer::GetLocalPlayer()->IsAlive() )
			return;// Not allowed to execute commands while alive
		else if( localPlayer && localPlayer->GetObserverMode() == OBS_MODE_DEATHCAM )
			return;// In the death cam spiral counts as alive
	}

	g_pMapOverview->SetMode( mode );
	cl_preferred_radar_mode.SetValue( mode );
}

////////////////////////////////////

DECLARE_HUDELEMENT_DEPTH( CHAJMapOverview, 60 );

using namespace vgui;

CHAJMapOverview::CHAJMapOverview( const char *pElementName ): BaseClass( pElementName )
{
	SetParent( g_pClientMode->GetViewport()->GetVPanel() );

	SetBounds( 0,0, 256, 256 );
	SetBgColor( Color( 0,0,0,100 ) );


	// Make sure we actually have the font...
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	
	m_hIconFont = pScheme->GetFont( "DefaultSmall" );

	SetBorder( pScheme->GetBorder("BaseBorder" ) );
	
	m_nMapTextureID = -1;
	m_MapKeyValues = NULL;

	m_MapOrigin = Vector( 0, 0, 0 );
	m_fMapScale = 1.0f;
	m_bFollowAngle = false;

	ShowPanel( true );

	m_fZoom = 3.0f;
	m_MapCenter = Vector2D( 512, 512 );
	m_ViewOrigin = Vector2D( 512, 512 );
	m_fViewAngle = 0;
	m_fTrailUpdateInterval = 1.0f;

	m_bShowNames = true;
	m_bShowHealth = true;
	m_bShowTrails = true;

	m_flChangeSpeed = 1000;
	//m_flIconSize = 64.0f;
	m_flIconSize = overview_player_size.GetFloat();

	m_ObjectCounterID = 1;

	Reset();
	
	Q_memset( m_Players, 0, sizeof(m_Players) );

	InitTeamColorsAndIcons();

	g_pMapOverview = this;  // for cvars access etc

	//SetMode( MAP_MODE_RADAR );
	//SetPlayerPreferredMode( MAP_MODE_RADAR );
	//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "MapZoomToSmall" );

	// get an ID for our padlock icon.
	m_iLockedIconID = FindOrCreateTexID( "hud/obj_overlay_locked" );

};

CHAJMapOverview::~CHAJMapOverview()
{
};


bool CHAJMapOverview::CanPlayerHealthBeSeen( MapPlayer_t *player )
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !localPlayer || !player )
		return false;

	if ( localPlayer->GetUserID() == (player->userid) )
		return false;

	return BaseClass::CanPlayerHealthBeSeen( player );
}

bool CHAJMapOverview::CanPlayerBeSeen(MapPlayer_t *player)
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !localPlayer || !player )
		return false;

	// do draw ourself
	if ( localPlayer->GetUserID() == (player->userid) )
		return true;

	// Invalid guy.
	if( player->position == Vector(0,0,0) )
		return false; 

	// if local player is on spectator team, he can see everyone
	if ( localPlayer->GetTeamNumber() <= TEAM_SPECTATOR )
		return true;

	// we never track unassigned or real spectators
	if ( player->team <= TEAM_SPECTATOR )
		return false;

	// don't show them if they are dead
	if( player->health <= 0 )
		return false;

	// if observer is an active player, check mp_forcecamera:
	if ( mp_forcecamera.GetInt() == OBS_ALLOW_NONE )
		return false;

	if ( mp_forcecamera.GetInt() == OBS_ALLOW_TEAM )
	{
		// true if both players are on the same team
		return (localPlayer->GetTeamNumber() == player->team );
	}

	// only show players on your team
	return (localPlayer->GetTeamNumber() == player->team );
}


void CHAJMapOverview::DrawObjectives()
{
	// sort flags by user order index
	CUtlMap<int, C_HajObjective*> sortedObjectives;
	sortedObjectives.SetLessFunc(IntLessFunc_Ascending);

	int nCPs = 0;
	const CUtlLinkedList<C_HajObjective*>& objectives = _objectiveman.GetObjectives();
	unsigned short it = objectives.Head();
	while(objectives.IsValidIndex(it))
	{
		C_HajObjective* pObjective = objectives.Element(it);

		int sortIndex = pObjective->GetSortIndex();
		sortedObjectives.Insert(sortIndex, pObjective);

		++nCPs;
		it = objectives.Next(it);
	}

	// nothing to draw unless we have capture points
	if(nCPs < 1)
		return;
	
	// set icon sizes
	int iconHeight = overview_objective_size.GetFloat() * ( ScreenHeight() / 480 );
	int iconWidth = iconHeight;
	int iconOffset = iconHeight/2;
	int iconX1 = 0;
	int iconY1 = 0;
	int iconX2 = iconX1 + iconWidth;
	int iconY2 = iconY1 + iconHeight;

	CUtlMap<int, C_HajObjective*>::IndexType_t itObj = sortedObjectives.FirstInorder();
	while(sortedObjectives.IsValidIndex(itObj))
	{
		C_HajObjective* pObjective = sortedObjectives.Element(itObj);
		
		// cast early so we can check to show it or not
		C_CapturePoint* pCapturePoint = dynamic_cast<C_CapturePoint*>(pObjective);
		
		if ( pObjective && pObjective->GetShowOnHud() == true )
		{
			// convert world origin to map origin
			Vector pos = pObjective->GetAbsOrigin();
			Vector2D pospanel = WorldToMap( pos );
			pospanel = MapToPanel( pospanel );

			iconX1 = pospanel.x - iconOffset;
			iconY1 = pospanel.y - iconOffset;
			iconX2 = pospanel.x + iconOffset;
			iconY2 = pospanel.y + iconOffset;

			// check if objective is even in the map view and if not skip it
			if ( !IsInPanel( pospanel ) )
			{
				itObj = sortedObjectives.NextInorder(itObj);
				continue;
			}

			// determine flag texture by team
			int iOwnerTeam = pObjective->GetOwnerTeamId();
			int ownerFlagTexId = GetFlagTextureId( pObjective, iOwnerTeam);
			
			// draw the owner icon
			surface()->DrawSetColor( 255, 255, 255, overview_objective_alpha.GetInt() );
			surface()->DrawSetTexture( ownerFlagTexId );
			surface()->DrawTexturedRect( iconX1, iconY1, iconX2, iconY2 );

			// draw padlock is cap is locked for your team
			/*C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();
			
			if ( pPlayer )
			{
				if ( ( pPlayer->GetTeamNumber() == TEAM_CWEALTH && pCapturePoint->IsCWealthLocked() ) ||
					 ( pPlayer->GetTeamNumber() == TEAM_AXIS && pCapturePoint->IsAxisLocked() ))
				{
					surface()->DrawSetColor( 255, 255, 255, 255 );
					surface()->DrawSetTexture( m_iLockedIconID );
					surface()->DrawTexturedRect( iconX1 + iconOffset, iconY1 + iconOffset, iconX2, iconY2 );
				}
			}*/
		}
		itObj = sortedObjectives.NextInorder(itObj);
	}
}


void CHAJMapOverview::SetMode( int mode )
{
	m_flChangeSpeed = 0; // change size instantly

	if ( mode == MAP_MODE_OFF )
	{
		ShowPanel( false );

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "MapOff" );
	}
	else if ( mode == MAP_MODE_INSET || mode == MAP_MODE_RADAR)
	{
		if( m_nMapTextureID == -1 )
		{
			SetMode( MAP_MODE_OFF );
			return;
		}

		if ( m_nMode != MAP_MODE_OFF )
			m_flChangeSpeed = 1000; // zoom effect

		C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();

		if( pPlayer && pPlayer->IsObserver() && pPlayer->GetObserverTarget() && pPlayer->GetObserverTarget()->IsPlayer() )
			pPlayer = ToBasePlayer( pPlayer->GetObserverTarget() );

		if ( pPlayer )
            SetFollowEntity( pPlayer->entindex() );

		ShowPanel( true );

		if ( mode != m_nMode && RunHudAnimations() )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "MapZoomToSmall" );
		}
	}
	else if ( mode == MAP_MODE_FULL )
	{
		if( m_nMapTextureID == -1 )
		{
			SetMode( MAP_MODE_OFF );
			return;
		}

		if ( m_nMode != MAP_MODE_OFF )
			m_flChangeSpeed = 1000; // zoom effect

		SetFollowEntity( 0 );

		ShowPanel( true );

		if ( mode != m_nMode && RunHudAnimations() )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "MapZoomToLarge" );
		}
	}

	// finally set mode
	m_nMode = mode;

	UpdateSizeAndPosition();
}

bool CHAJMapOverview::ShouldDraw( void )
{
	if( HajGameRules() && HajGameRules()->IsGameOver() )
		return false;

	if( cl_drawradar.GetBool() == true )
	{
		return true;
	}

	return false;
}


void CHAJMapOverview::OnThink( void )
{
	SetAlpha( cl_radar_alpha.GetInt() );
	m_flIconSize = overview_player_size.GetFloat();

	BaseClass::OnThink();
}

void CHAJMapOverview::Paint()
{
	UpdateSizeAndPosition();
	UpdateFollowEntity();
	UpdatePlayers();
	UpdatePlayerTrails();

	DrawMapTexture();
	DrawObjectives();
	DrawMapPlayerTrails();
	DrawMapPlayers();
	DrawCamera();
	//BaseClass::Paint();
}

int CHAJMapOverview::FindOrCreateTexID( char* texname )
{
	// return the texture ID for the named texture or if we cant find it, create one.
	int id = vgui::surface()->DrawGetTextureId( texname );
	if ( id == -1 )
	{
		id = vgui::surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( id, texname, false, true );
	}
	return id;
}


int CHAJMapOverview::GetFlagTextureId( C_HajObjective* pObjective, int teamId )
{
	int textureId = -1;

	// get the texture ID for the hud icon texture for this objective
	switch( teamId )
	{
		case TEAM_AXIS:
			if ( !Q_stricmp( pObjective->GetAxisIcon(), "" ) )
				textureId = FindOrCreateTexID( "hud/obj_axis_default" );
			else
				textureId = FindOrCreateTexID( pObjective->GetAxisIcon() );
			break;

		case TEAM_CWEALTH:
			if ( !Q_stricmp( pObjective->GetAlliesIcon(), "" ) )
				textureId = FindOrCreateTexID( "hud/obj_allies_default" );
			else
				textureId = FindOrCreateTexID( pObjective->GetAlliesIcon() );
			break;
		
		default:
			if ( !Q_stricmp( pObjective->GetNeutralIcon(), "" ) )
				textureId = FindOrCreateTexID( "hud/obj_neutral_default" );
			else
				textureId = FindOrCreateTexID( pObjective->GetNeutralIcon() );
			break;
	}

	return textureId;
};

void CHAJMapOverview::FireGameEvent( IGameEvent *event )
{
	BaseClass::FireGameEvent( event );

	const char * type = event->GetName();

	if ( Q_strcmp(type, "game_newmap") == 0 )
	{
		SetMode( cl_preferred_radar_mode.GetInt() );
		Reset();
	}

}

void CHAJMapOverview::InitTeamColorsAndIcons()
{
	BaseClass::InitTeamColorsAndIcons();

	m_TeamColors[ TEAM_CWEALTH ] = COLOR_GREEN;
	m_TeamIcons[ TEAM_CWEALTH ] = AddIconTexture( "hud/hud_brit_player" );

	m_TeamColors[ TEAM_AXIS ] = COLOR_RED;
	m_TeamIcons[ TEAM_AXIS ] = AddIconTexture( "hud/hud_ger_player" );

	m_TeamColors[ 0 ] = COLOR_YELLOW;
	m_TeamIcons[ 0 ] = AddIconTexture( "hud/hud_local_player" );
}