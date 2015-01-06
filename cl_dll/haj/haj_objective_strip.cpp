//========= Copyright © 2011, Ham and Jam. ==============================//
// Purpose: HUD element to show the objectives
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_misc.h"
#include "haj_objectivemanager_c.h"
#include "haj_capturepoint_c.h"
#include "haj_objective_strip.h"

#include "c_playerresource.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudObjectiveStrip );

// icons for objective roles
const char* g_ObjectiveIcons[OBJECTIVE_ROLE_UNKNOWN+1] =
{
	NULL,								// OBJECTIVE_ROLE_CAPTURE = 0,
	"hud/obj_overlay_destroy",			// OBJECTIVE_ROLE_PLANT_EXPLOSIVES,
	"hud/obj_overlay_defuse",			// OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES,
	"hud/obj_overlay_defend",			// OBJECTIVE_ROLE_PREVENT_DEFUSE,
	"hud/obj_overlay_defend",			// OBJECTIVE_ROLE_DEFEND,
	"hud/obj_overlay_defend",			// OBJECTIVE_ROLE_HOLD_POSITION,
	"hud/obj_overlay_locked",			// OBJECTIVE_ROLE_LOCKED,
	NULL,								// OBJECTIVE_ROLE_COMPLETED,
	"hud/obj_overlay_destroyed",		// OBJECTIVE_ROLE_DESTROYED,
	NULL,								// OBJECTIVE_ROLE_UNKNOWN
};

CHudObjectiveStrip::CHudObjectiveStrip( const char *pElementName ) : BaseClass(NULL, "HudObjectiveStrip"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent ); 

	SetPaintBackgroundEnabled(true);

	m_sortedObjectives.SetLessFunc(IntLessFunc_Ascending);

}

//-----------------------------------------------------------------------------
// Purpose: Apply VGUI scheme
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pPanelBackground = gHUD.GetIcon( "hud_obj_bg" );
	m_pAlertIcon = gHUD.GetIcon( "obj_flash" );

	// get role icons
	for( int i = 0; i < OBJECTIVE_ROLE_UNKNOWN; i++ )
	{
		if( g_ObjectiveIcons[i] != NULL )
			m_iStatusIcons[i] = FindOrCreateTexID( g_ObjectiveIcons[i] );
		else
			m_iStatusIcons[i] = -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Paint the background
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::PaintBackground()
{
	float flProportionalWidth = (float)m_pPanelBackground->Width() * ( (float)m_iBackgroundHeight / (float)m_pPanelBackground->Height() );
	int iBgX = (-m_iIconInsetX) + ( m_sortedObjectives.Count() * ( m_iIconWidth + m_iIconSpacing ) ); 

	m_pPanelBackground->DrawSelf( iBgX, 0, (int)flProportionalWidth, m_iBackgroundHeight, Color( 255, 255, 255, 255 ) );

	DrawRoundTimer( iBgX );
}

//-----------------------------------------------------------------------------
// Purpose: Draw the time remaining in the round
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::DrawRoundTimer( int iBgX )
{
	CHajGameRules* pGameRules = HajGameRules();

	if( pGameRules )
	{
		float timeleft = pGameRules->GetRoundTimeLeft();

		int nMinutes = timeleft / 60;
		int nSeconds = (int)timeleft % 60;

		wchar_t wszTimeLeft[150] = L"";
		_snwprintf( wszTimeLeft, sizeof( wszTimeLeft ), L"%02d:%02d", nMinutes, nSeconds );

		int tlw, tlh;
		vgui::surface()->GetTextSize( m_hTimeFont, wszTimeLeft, tlw, tlh );

		float iDrawX = ( iBgX + m_iTimerX ) - ( tlw / 2 );

		vgui::surface()->DrawSetTextFont( m_hTimeFont );
		vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, 255 ) );
		vgui::surface()->DrawSetTextPos( ceil(iDrawX), m_iTimerY );
		vgui::surface()->DrawUnicodeString( wszTimeLeft );

	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw objectives here
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::Paint()
{
	PaintBackground();

	if( m_LocalPlayer == NULL )
		return;

	CUtlMap<int, CHandle<C_HajObjective> >::IndexType_t itObj = m_sortedObjectives.FirstInorder();
	int iCurX = m_iIconSpacing * 2;

	int iTooltipX = -1;
	C_HajObjective *pHighlightedObjective = NULL;

	while( m_sortedObjectives.IsValidIndex(itObj) )
	{
		C_HajObjective* pCapturePoint = m_sortedObjectives.Element(itObj);

		if( pCapturePoint && pCapturePoint->GetShowOnHud() )
		{
			int ownerFlagTexId = GetFlagTextureId( pCapturePoint, pCapturePoint->GetOwnerTeamId() );

			// draw owner team's flag
			vgui::surface()->DrawSetColor(255, 255, 255, 255);
			vgui::surface()->DrawSetTexture(ownerFlagTexId);
			vgui::surface()->DrawTexturedRect( iCurX, m_iIconY, iCurX + m_iIconWidth, m_iIconY + m_iIconWidth ); // it's assumed that icons are 1:1

			int iCapturingTeam = pCapturePoint->GetCapturingTeamId();

			// capture progress bar
			if( iCapturingTeam > -1 && pCapturePoint->GetCapturePercentile() > 0.0f )
			{
				float percentile = pCapturePoint->GetCapturePercentile();
				int capturingFlagTexId = GetFlagTextureId(pCapturePoint, iCapturingTeam);

				vgui::surface()->DrawSetColor(255, 255, 255, 255);
				vgui::surface()->DrawSetTexture(capturingFlagTexId);
				vgui::surface()->DrawTexturedSubRect( iCurX, m_iIconY, iCurX + floor( m_iIconWidth * percentile),m_iIconY + m_iIconWidth, 0.0f, 0.0f, percentile, 1.0f);
			}

			if( pCapturePoint->IsLocalPlayerInArea() || ( pCapturePoint == m_HighlightedObjective && !pHighlightedObjective ) )
			{
				iTooltipX = iCurX;
				pHighlightedObjective = pCapturePoint;
			}

			DrawObjectiveExtras( pCapturePoint, iCurX );
		}

		iCurX += m_iIconSpacing + m_iIconWidth;

		// next obj
		itObj = m_sortedObjectives.NextInorder(itObj);
	}

	// draw tooltip
	if( pHighlightedObjective )
	{
		DrawExtendedInfo( pHighlightedObjective, iTooltipX );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw objective specifics
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::DrawObjectiveExtras( C_HajObjective *pObjective, int iXPos )
{

	// have flashing beacon to alert players of capture
	if( pObjective->ShouldDrawAlert() )
	{
		// alert colour
		float alpha = 20.0f + ( ( sin( gpGlobals->curtime * 5 ) + 1 ) * 30.0 );
		Color colAlertColor = ( ( pObjective->GetCapturingTeamId() == m_LocalPlayer->GetTeamNumber() ) ? Color( 92, 126, 200, alpha ) : Color( 255, 0, 0, alpha ) );

		m_pAlertIcon->DrawSelf( iXPos, m_iIconY, m_iIconWidth, m_iIconWidth, colAlertColor );
	}

	// draw status icon (locked etc)
	int iStatusIcon = m_iStatusIcons[pObjective->GetTeamRole( m_LocalPlayer->GetTeamNumber() )];

	pObjective->PostDrawIcon( iXPos, m_iIconY, m_iIconWidth, m_iIconWidth, YRES(2) );

	if( iStatusIcon > -1 )
	{
		int x = iXPos + m_iIconWidth - (m_iStatusIconSize/2) - m_iStatusIconX;
		int y = m_iIconY + m_iIconWidth - (m_iStatusIconSize/2) - m_iStatusIconY;

		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		vgui::surface()->DrawSetTexture( iStatusIcon );
		vgui::surface()->DrawTexturedRect( x, y, x + m_iStatusIconSize, y + m_iStatusIconSize );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw objective tooltip (local player in area, highlighted on HUD)
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::DrawExtendedInfo( C_HajObjective *pObjective, int iXPos )
{
	const char* szObjectiveRole = pObjective->GetStringForRole();
	wchar_t wszObjectiveDescription[250];

	// we have a descriptive role at the capture point, print the details.
	if( szObjectiveRole != NULL )
	{
		_snwprintf( wszObjectiveDescription, sizeof( wszObjectiveDescription ), L"%s", vgui::localize()->Find( szObjectiveRole ) );
	}
	// or just show the player count
	else
	{
		int iCount = 0;
		int iNeeded = pObjective->GetCapCountByTeam( m_LocalPlayer->GetTeamNumber() );

		wchar_t wszPlayerList[250];

		for( int i = 0; i < MAX_PLAYERS; i++ )
		{
			EHANDLE hPlayer = pObjective->m_playersInArea[i];

			if( hPlayer )
			{
				C_BasePlayer *pOccupier = dynamic_cast<C_BasePlayer*>((C_BaseEntity*)hPlayer);

				if( pOccupier )
				{
					if( pOccupier->GetTeamNumber() == m_LocalPlayer->GetTeamNumber() )
					{
						if( iCount == 0 )
							_snwprintf( wszPlayerList, sizeof( wszPlayerList ), L"%S", pOccupier->GetPlayerName() );
						else
							_snwprintf( wszPlayerList, sizeof( wszPlayerList ), L"%s, %S", wszPlayerList, pOccupier->GetPlayerName() );

						iCount++;
					}
				}
			}
		}

		if( iCount == 0 ) // no players
			vgui::localize()->ConstructString( wszObjectiveDescription, sizeof(wszObjectiveDescription), vgui::localize()->Find( "#HaJ_NoPlayers" ), 0 );
		else
			vgui::localize()->ConstructString( wszObjectiveDescription, sizeof(wszObjectiveDescription), vgui::localize()->Find( "#HaJ_CapturePlayers" ), 1, wszPlayerList );


		// required players
		int iNeeds = iNeeded - iCount;

		if( iNeeds > 0 )
		{
			wchar_t wszNeed[75];
			wchar_t wszNeedsTemp[5];

			_snwprintf( wszNeedsTemp, sizeof( wszNeedsTemp ), L"%d", iNeeds );

			if( iNeeds == 1 )
				vgui::localize()->ConstructString( wszNeed, sizeof(wszNeed), vgui::localize()->Find( "#HaJ_CaptureNeedPlayer" ), 0 );		
			else
				vgui::localize()->ConstructString( wszNeed, sizeof(wszNeed), vgui::localize()->Find( "#HaJ_CaptureNeedPlayers" ), 1, wszNeedsTemp );	

			_snwprintf( wszObjectiveDescription, sizeof( wszObjectiveDescription ), L"%s%s", wszObjectiveDescription, wszNeed );							
		}
	}

	wchar_t wszCaptureName[150];
	wchar_t *pszCaptureNameLocal = vgui::localize()->Find( pObjective->GetNameOfZone() );

	if( !pszCaptureNameLocal ) _snwprintf( wszCaptureName, sizeof( wszCaptureName ), L"%S", pObjective->GetNameOfZone() );
	else _snwprintf( wszCaptureName, sizeof( wszCaptureName ), L"%s", pszCaptureNameLocal );

	int iCapWidth, iCapHeight, iInfoWidth, iInfoHeight;
	vgui::surface()->GetTextSize( m_TooltipCapNameFont, wszCaptureName, iCapWidth, iCapHeight );
	vgui::surface()->GetTextSize( m_TooltipInfoFont, wszObjectiveDescription, iInfoWidth, iInfoHeight );

	int iTooltipWidth = max( ((iCapWidth > iInfoWidth) ? iCapWidth : iInfoWidth ), m_iIconWidth ) + (m_iTooltipInset*2);
	int iTooltipHeight = (m_iTooltipInset*3) + iCapHeight + iInfoHeight;

	// draw background for info
	vgui::surface()->DrawSetColor( m_TooltipBackground );
	vgui::surface()->DrawFilledRect( iXPos, m_iTooltipY, iXPos + iTooltipWidth, m_iTooltipY + iTooltipHeight );

	// draw it
	Color colTitleColor = g_PR->GetTeamColor( pObjective->GetOwnerTeamId() );

	vgui::surface()->DrawSetTextFont( m_TooltipCapNameFont );
	vgui::surface()->DrawSetTextColor( colTitleColor );
	vgui::surface()->DrawSetTextPos( iXPos + m_iTooltipInset, m_iTooltipY + m_iTooltipInset );
	vgui::surface()->DrawUnicodeString( wszCaptureName );

	// draw description
	vgui::surface()->DrawSetTextFont( m_TooltipInfoFont );
	vgui::surface()->DrawSetTextColor( m_TooltipInfoColor );
	vgui::surface()->DrawSetTextPos( iXPos + m_iTooltipInset, m_iTooltipY + (m_iTooltipInset*2) + iCapHeight - m_iTooltipInfoOffsetY );
	vgui::surface()->DrawUnicodeString( wszObjectiveDescription );
}

//-----------------------------------------------------------------------------
// Purpose: Find objectives in the map and sort them, ready to be drawn
//-----------------------------------------------------------------------------
void CHudObjectiveStrip::Think()
{
	m_sortedObjectives.RemoveAll();

	const CUtlLinkedList<C_HajObjective*>& objectives = _objectiveman.GetObjectives();
	unsigned short it = objectives.Head();

	while(objectives.IsValidIndex(it))
	{
		C_HajObjective* pObjective = objectives.Element(it);

		int sortIndex = pObjective->GetSortIndex();
		m_sortedObjectives.Insert(sortIndex, pObjective);

		it = objectives.Next(it);
	}

	// determine local player (or the spectated player)
	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( pLocalPlayer && pLocalPlayer->IsObserver() && pLocalPlayer->GetObserverTarget() )
		pLocalPlayer = ToHajPlayer( pLocalPlayer->GetObserverTarget() );

	m_LocalPlayer = pLocalPlayer;
}

/////////////////////////////////////////////////////////////////////////////
int CHudObjectiveStrip::FindOrCreateTexID( const char* texname )
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

/////////////////////////////////////////////////////////////////////////////
int CHudObjectiveStrip::GetFlagTextureId( C_HajObjective* pObjective, int teamId )
{
	char textureName[ MAX_PATH ];
	Q_snprintf( textureName, sizeof( textureName ), pObjective->GetTeamIcon( teamId ) );

	// get the texture ID for the hud icon texture for this objective
	return FindOrCreateTexID( textureName );
}

//-----------------------------------------------------------------------------
// Purpose: Draw this panel?
//-----------------------------------------------------------------------------
bool CHudObjectiveStrip::ShouldDraw()
{
	if( HajGameRules() )
		return ( CHudElement::ShouldDraw() && !HajGameRules()->IsIntermission() && !(HajGameRules()->IsFreeplay() || HajGameRules()->IsGameOver()) );

	return false;
}