//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ Objective location class
// Notes:    
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "utlvector.h"
#include "view.h"

#include "c_playerresource.h"
#include "haj_capturepoint_c.h"
#include "haj_objective_strip.h"
#include "haj_gamerules.h"
#include "haj_misc.h"
#include "haj_objectivemanager_c.h"
#include "haj_player_c.h"

#include "tier0/memdbgon.h"

extern ConVar overview_objective_size;
extern bool g_bDisplayObjectiveLocations;

ConVar cl_drawobjectives( "cl_drawobjectives", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Show objectives on HUD" );
ConVar cl_drawobjectivedistance( "cl_drawobjectivedistance", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Draw target ID for objective (distance). 0 = off, 1 = metric, 2 = imperial" );
ConVar cl_drawobjectivesalpha( "cl_drawobjectivesalpha", "150", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Alpha to draw objectives at" );
ConVar cl_drawobjectivesalphafocus( "cl_drawobjectivesalphafocus", "35", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Alpha to draw objectives at when they're in the middle of the screen" );


// class definitions
class CHAJObjectiveLocation : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHAJObjectiveLocation, vgui::Panel);

public:
	CHAJObjectiveLocation(const char *pElementName);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Init();
	virtual bool ShouldDraw();
	virtual void Paint();
	virtual void Reset();
	virtual void VidInit();

	virtual bool DrawOverlayIconForObjective( C_HajObjective *pObjective, int iconX1, int iconY1, int iconX2, int iconY2, int iconOffset ); // if true is returned it's in the middle of the screen
	
protected:

	void PaintString(vgui::HFont hFont, int nXLoc, int nYLoc, wchar_t *czText);
	int	 FindOrCreateTexID( char* texname );
	int  GetFlagTextureId( C_HajObjective* pObjective, int teamId );

	CPanelAnimationVar( Color, m_ObjectiveFontColor, "ObjectiveFontColor", "255 255 255 255");
	CPanelAnimationVar( vgui::HFont, m_hObjectiveFont, "ObjectiveFont", "CaptureDetail");

	Color m_ObjectiveNormalColor;
	Color m_PauseCaptureColor;
	Color m_OpposingCaptureColor;
	Color m_FriendlyCaptureColor;

	// Objective Positioning
	CPanelAnimationVarAliasType(float, m_fObjectiveBlockSize, "objective_size", "24", "proportional_float" );
};

// class macro
// comment out to disable it for now
DECLARE_HUDELEMENT_DEPTH( CHAJObjectiveLocation, 20 );

//=====================
//CHAJObjectiveLocation
//=====================

// Constructor & Deconstructor
CHAJObjectiveLocation::CHAJObjectiveLocation( const char *pElementName) : CHudElement( pElementName ), BaseClass( NULL, "HAJObjectiveLocation" )
{
	// Set visibility rules and the parent.
	SetHiddenBits( HIDEHUD_MISCSTATUS );
	SetParent( g_pClientMode->GetViewport() );
	
	// turn this off when your using for real
	SetVisible( false );
	SetPaintBackgroundEnabled( false );
	SetProportional( true );
	SetScheme( "ClientScheme" );

	// Setup colors
	m_ObjectiveNormalColor = Color(255,255,255,255);
	m_PauseCaptureColor = Color(255,255,0,255);
	m_OpposingCaptureColor = Color(255,0,0,255);
	m_FriendlyCaptureColor = Color(0,255,0,255);
}


void CHAJObjectiveLocation::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CHAJObjectiveLocation::Init()
{
}

void CHAJObjectiveLocation::Reset()
{
}

void CHAJObjectiveLocation::VidInit()
{
}

bool CHAJObjectiveLocation::ShouldDraw()
{
	if( HajGameRules() && ( HajGameRules()->IsIntermission() || HajGameRules()->IsFreeplay() || HajGameRules()->IsGameOver() ) ) 
		return false;

	return cl_drawobjectives.GetBool();
}

void CHAJObjectiveLocation::Paint()
{
	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();

	int iLocalTeam = pLocalPlayer->GetTeamNumber();

	if( pLocalPlayer->IsObserver() && (pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE || pLocalPlayer->GetObserverMode() == OBS_MODE_CHASE ) && pLocalPlayer->GetObserverTarget() )
		iLocalTeam = pLocalPlayer->GetObserverTarget()->GetTeamNumber();

	// Set panel size to hud size
	int nHudWidth, nHudHeight;
	GetHudSize( nHudWidth, nHudHeight );
	SetSize( nHudWidth, nHudHeight );	// sets size of panel

	// Set the text color
	vgui::surface()->DrawSetTextColor( m_ObjectiveFontColor );

	// Initial paint state.
	SetPaintBackgroundEnabled(false);

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
	int iconHeight = overview_objective_size.GetFloat();
	int iconWidth = iconHeight;
	int iconOffset = iconHeight/2 - 50;
	int iconX1 = 0;
	int iconY1 = 0;
	int iconX2 = iconX1 + iconWidth;
	int iconY2 = iconY1 + iconHeight;

	C_HajObjective *pCenterObjective = NULL;
	bool bPlayerIsInCap = false;

	CUtlMap<int, C_HajObjective*>::IndexType_t itObj = sortedObjectives.FirstInorder();
	while(sortedObjectives.IsValidIndex(itObj))
	{
		C_HajObjective* pObjective = sortedObjectives.Element(itObj);

		if( pObjective )
		{
			if( !bPlayerIsInCap )
				bPlayerIsInCap = pObjective->IsLocalPlayerInArea();

			int iTeamRole = pObjective->GetTeamRole(iLocalTeam);
			if( pObjective->GetShowOnHud() && !( iTeamRole == OBJECTIVE_ROLE_COMPLETED || iTeamRole == OBJECTIVE_ROLE_LOCKED ) )
			{	
				if ( DrawOverlayIconForObjective( pObjective , iconX1, iconY1, iconX2, iconY2, iconOffset ) && !bPlayerIsInCap && pCenterObjective == NULL )
					pCenterObjective = pObjective;
			}
		}

		itObj = sortedObjectives.NextInorder(itObj);
	}

	// target id for objective in center of screen
	CHudObjectiveStrip *pObjectiveStrip = GET_HUDELEMENT(CHudObjectiveStrip);

	if( pObjectiveStrip && !bPlayerIsInCap )
		pObjectiveStrip->SetHighlightedObjective( pCenterObjective );
	else
		pObjectiveStrip->SetHighlightedObjective( NULL );

	if( cl_drawobjectivedistance.GetBool() && pCenterObjective != NULL )
	{	
		wchar_t caption[120];
		const wchar_t* unit = L"";
		float coefficient = 1.0f;

		if( cl_drawobjectivedistance.GetInt() == 2 )
		{
			unit = L"yd";
			coefficient = 0.02777f;
		}
		else
		{
			unit = L"m";
			coefficient = 0.0254f;
		}

		int distance = (int)floor( ( pCenterObjective->GetRenderOrigin() - MainViewOrigin() ).Length() * coefficient );

		_snwprintf( caption, sizeof( caption ), L"%d %s", distance, unit );

		int w, h;
		vgui::surface()->GetTextSize( m_hObjectiveFont, caption, w, h );

		int x = (ScreenWidth()/2) - (w/2);
		int y = (ScreenHeight()*0.57);

		Color c = g_PR->GetTeamColor( pCenterObjective->GetOwnerTeamId() );
		c[3] = 255;

		vgui::surface()->DrawSetTextFont( m_hObjectiveFont );
		vgui::surface()->DrawSetTextColor( c );
		vgui::surface()->DrawSetTextPos( x, y );
		vgui::surface()->DrawUnicodeString( caption );
	}

}



bool CHAJObjectiveLocation::DrawOverlayIconForObjective( C_HajObjective *pObjective, int iconX1, int iconY1, int iconX2, int iconY2, int iconOffset )
{
	int nXS = 0, nYS = 0;
	Color renderColor( m_ObjectiveNormalColor );
	bool bInCenter = false;

	// early culling if its not on the screen
	if ( GetVectorInScreenSpace( pObjective->GetAbsOrigin() + Vector( 0, 0, 80 ), nXS, nYS, 0) )
	{
		float flDistance = ( pObjective->GetRenderOrigin() - MainViewOrigin() ).Length();

		// determine flag texture by team
		int iOwnerTeam = pObjective->GetOwnerTeamId();
		int ownerFlagTexId = GetFlagTextureId( pObjective, iOwnerTeam);

		iconX1 = nXS - iconOffset;
		iconY1 = nYS - iconOffset;
		iconX2 = nXS + iconOffset;
		iconY2 = nYS + iconOffset;

		float alpha = renderColor[3];

		if ( flDistance < 555 ) // far away?
		{
			if( flDistance >= 300 )
				alpha = ( ( flDistance - 300 ) / 255 ) * alpha;
			else
				alpha = 0;
		}
		else
		{
			float t_alpha = (float)clamp( ( (flDistance - 2048.0f ) * 0.1245f ), 20, cl_drawobjectivesalpha.GetInt() );
			alpha = clamp( 255 - t_alpha, 10, 150 );
		}

		int distanceOriginX = ScreenWidth() / 2;
		int distanceOriginY = ScreenHeight() / 2;

		int dX = distanceOriginX - nXS;
		int dY = distanceOriginY - nYS;
		float distance = sqrt( (float)(dX*dX + dY*dY) );
		float scaler = ( ScreenWidth() / 800 );

		if( distance < 40 * scaler)
		{
			if( alpha > cl_drawobjectivesalphafocus.GetInt() )
				alpha = cl_drawobjectivesalphafocus.GetInt();

			bInCenter = true;
		}

		renderColor[3] = alpha;

		// draw the owner icon
		vgui::surface()->DrawSetColor( renderColor );
		vgui::surface()->DrawSetTexture( ownerFlagTexId );

		// Notify the user who currently owns this objective.
		vgui::surface()->DrawTexturedSubRect( nXS - 16, nYS - 16, nXS + 16, nYS + 16,  0.0f, 0.0f, 1.0f, 1.0f );
	}

	return bInCenter;
}

void CHAJObjectiveLocation::PaintString(vgui::HFont hFont, int nXLoc, int nYLoc, wchar_t *czText )
{
	// Set the font and position then draw the character.
	vgui::surface()->DrawSetTextFont(hFont);
	vgui::surface()->DrawSetTextPos(nXLoc, nYLoc);

	// Draw the string.
	for (wchar_t *cChar = czText; *cChar != 0; cChar++)
		vgui::surface()->DrawUnicodeChar(*cChar);
}


int CHAJObjectiveLocation::FindOrCreateTexID( char* texname )
{
	// return the texture ID for the named texture or if we cant find it, create one.
	int id = vgui::surface()->DrawGetTextureId( texname );
	if ( id == -1 )
	{
		id = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( id, texname, false, true );
	}
	return id;
}


int CHAJObjectiveLocation::GetFlagTextureId( C_HajObjective* pObjective, int teamId )
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
}