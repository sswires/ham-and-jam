
// haj_hudcapturepoints.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_hudcapturepoints.h"
#include "haj_capturepoint_c.h"
#include "haj_gamerules.h"
#include "haj_misc.h"
#include "haj_objectivemanager_c.h"
#include "haj_player_c.h"

#include "clientmode_hl2mpnormal.h"

#include "c_team.h"
#include "c_playerresource.h"

#include "iclientmode.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// game over?
extern bool g_fGameOver;

/////////////////////////////////////////////////////////////////////////////
DECLARE_HUDELEMENT(CHudCapturePoint);

/////////////////////////////////////////////////////////////////////////////
CHudCapturePoint::CHudCapturePoint(const char* szElementName)
: CHudElement(szElementName)
, BaseClass(NULL, "HudCapturePoints")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetProportional(true);
	SetHiddenBits( 0 );

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	m_hTitleFont = pScheme->GetFont("CaptureTitle", IsProportional() );
	m_hDetailFont = pScheme->GetFont( "CaptureDetail", IsProportional() );
	m_hPlayerFont = pScheme->GetFont( "CapturePLCount", IsProportional() );
}

/////////////////////////////////////////////////////////////////////////////
CHudCapturePoint::~CHudCapturePoint()
{

}

/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);

	SetWide( ScreenWidth() - 20 );

	// Make sure we actually have the font...
}

/////////////////////////////////////////////////////////////////////////////
int CHudCapturePoint::FindOrCreateTexID( char* texname )
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
int CHudCapturePoint::GetFlagTextureId( C_HajObjective* pObjective, int teamId )
{
	int textureId = -1;
	char textureName[ MAX_PATH ];

	Q_snprintf( textureName, sizeof( textureName ), pObjective->GetTeamIcon( teamId ) );

	// get the texture ID for the hud icon texture for this objective
	return FindOrCreateTexID( textureName );
}


/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::Init()
{

}

//-----------------------------------------------------------------------------
// Purpose: Draw if we've got at least one death notice in the queue
//-----------------------------------------------------------------------------
bool CHudCapturePoint::ShouldDraw( void )
{
	if( HajGameRules() )
		return ( CHudElement::ShouldDraw() && !HajGameRules()->IsIntermission() && !(HajGameRules()->IsFreeplay() || HajGameRules()->IsGameOver()) );
	
	return CHudElement::ShouldDraw();
}


/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::OnThink()
{
	SetPos( 10, GetClientModeHL2MPNormal()->GetDeathMessageStartHeight() + YRES(6) );
}

extern ConVar cl_drawobjectives;
/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::Paint()
{
	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( pLocalPlayer && ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR || pLocalPlayer->IsObserver() ) )
	{
		C_HajPlayer* pNewLocalPlayer = ToHajPlayer( pLocalPlayer->GetObserverTarget() );

		if( pNewLocalPlayer )
			pLocalPlayer = pNewLocalPlayer;
	}

	// get the render surface
	vgui::ISurface* pSurface = vgui::surface();
	assert(NULL != pSurface);

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

	// draw a textured rect for each capture point
	float scaler = GetTall()/36.0f;

	// background margin (proportional)
	int iconMargin = m_iBackgroundMargin;

	if( !m_pIconBackground )
		iconMargin = 0;

	// set padding between hud elements
	int padding = (int)5 * scaler;
	int iconHeight = (int)32 * scaler;
	int iconWidth = iconHeight;
	int x = 0;
	int y = 0;

	int iTeam = pLocalPlayer->GetTeamNumber();

	CUtlMap<int, C_HajObjective*>::IndexType_t itObj = sortedObjectives.FirstInorder();
	while(sortedObjectives.IsValidIndex(itObj))
	{
		C_HajObjective* pCapturePoint = sortedObjectives.Element(itObj);
		
		// cast early so we can check to show it or not
		C_CapturePoint* pCaptureArea = dynamic_cast<C_CapturePoint*>(pCapturePoint);
		
		if( !pCapturePoint )
			continue;

		C_HajPlayer *pPlayer = pLocalPlayer;
		bool bLocalPlayerInArea = false;

		if ( pPlayer )
		{
			bLocalPlayerInArea = ( pCapturePoint->IsLocalPlayerInArea() || (cl_drawobjectives.GetBool() && pCapturePoint == m_pHighlightedObjective ) );
		}

		if ( pCapturePoint->GetShowOnHud() == true )
		{
			if( bLocalPlayerInArea )
			{
				pSurface->DrawSetColor(0, 0, 0, 100);
				pSurface->DrawFilledRect( x, y, x + iconWidth + padding + ( 180 * scaler ) + 5, y + iconHeight );

				pCapturePoint->PostDrawInfoBar( x, y, x + iconWidth + padding + ( 180 * scaler ) + 5, y + iconHeight );
			}

			// determine flag texture by team
			int iOwnerTeam = pCapturePoint->GetOwnerTeamId();
			int ownerFlagTexId = GetFlagTextureId( pCapturePoint,  iOwnerTeam);
		
			// draw background
			if( iconMargin > 0 )
				m_pIconBackground->DrawSelf( x, y, iconWidth, iconHeight, Color( 255, 255, 255, 255 ) );

			// draw owner team's flag
			pSurface->DrawSetColor(255, 255, 255, 255);
			pSurface->DrawSetTexture(ownerFlagTexId);
			pSurface->DrawTexturedRect(x + iconMargin, y + iconMargin, x + iconWidth - iconMargin, y + iconHeight - iconMargin);

			// draw destroyed overlay
			if( pCapturePoint->GetTeamRole( pLocalPlayer->GetTeamNumber() ) == OBJECTIVE_ROLE_DESTROYED )
			{
				pSurface->DrawSetTexture( m_iDestroyedIconID );
				pSurface->DrawTexturedRect(x + iconMargin, y + iconMargin, x + iconWidth - iconMargin, y + iconHeight - iconMargin);
			}

			// hook for extra drawing after icon
			pCapturePoint->PostDrawIcon( x, y, iconWidth - iconMargin, y + iconHeight - iconMargin, iconMargin );

			// draw capturing team's flag, which also functions
			// like a progress bar
		
			int iCapturingTeam = pCapturePoint->GetCapturingTeamId();
			bool bDrawAlert = false;
			

			if((iCapturingTeam > -1) && (iCapturingTeam != iOwnerTeam) && pCapturePoint->GetCapturePercentile() > 0.0f )
			{
				float percentile = pCapturePoint->GetCapturePercentile();

				int capturingFlagTexId = GetFlagTextureId(pCapturePoint, iCapturingTeam);

				pSurface->DrawSetColor(255, 255, 255, 255);
				pSurface->DrawSetTexture(capturingFlagTexId);
				pSurface->DrawTexturedSubRect(x + iconMargin, y + iconMargin, x + floor( ( iconWidth - iconMargin ) * percentile), y + iconHeight - iconMargin,
											  0.0f, 0.0f, percentile, 1.0f);

				if( pCapturePoint->GetOccupantsByTeam( iCapturingTeam ) >= pCapturePoint->GetCapCountByTeam( iCapturingTeam ) )
					bDrawAlert = true;

			}
			else if( ( !pCapturePoint->IsLockedForTeam( iTeam ) && pCapturePoint->GetOwnerTeamId() != iTeam && pCapturePoint->GetOccupantsByTeam( iTeam ) > 0 && pCapturePoint->GetOccupantsByTeam( iTeam ) < pCapturePoint->GetCapCountByTeam( iTeam ) ) || pCapturePoint->GetTeamRole( pPlayer->GetTeamNumber() ) == OBJECTIVE_ROLE_PREVENT_DEFUSE )
			{
				if( m_pAlertIcon )
				{
					float alpha = 20.0f + ( ( sin( gpGlobals->curtime * 5 ) + 1 ) * 30.0 );
					m_pAlertIcon->DrawSelf( x + iconMargin, y + iconMargin, iconWidth - ( iconMargin * 2 ), iconHeight - ( iconMargin * 2 ), Color( 92, 126, 200, alpha ) );
				}
			}

			if( m_pAlertIcon && ( bDrawAlert || pCapturePoint->GetTeamRole( pPlayer->GetTeamNumber() ) == OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES ) )
			{
				float alpha = 20.0f + ( ( sin( gpGlobals->curtime * 5 ) + 1 ) * 30.0 );
				m_pAlertIcon->DrawSelf( x + iconMargin, y + iconMargin, iconWidth - ( iconMargin * 2 ), iconHeight - ( iconMargin * 2 ), Color( 255, 0, 0, alpha ) );
			}
			
			// draw padlock is cap is locked for your team
			if ( pPlayer )
			{
				int role = pCapturePoint->GetTeamRole( pPlayer->GetTeamNumber() );

				int iIconID = -1;
				bool bDrawNumPeople = false;

				pSurface->DrawSetColor(255, 255, 255, 255);

				// which texture to use????
				switch( role )
				{
					case OBJECTIVE_ROLE_DEFEND: 
						iIconID = m_iDefendIconID;
						break;

					case OBJECTIVE_ROLE_LOCKED:
						iIconID = m_iLockedIconID;
						break;

					case OBJECTIVE_ROLE_DEFUSE_EXPLOSIVES:
					case OBJECTIVE_ROLE_PREVENT_DEFUSE:
						iIconID = m_iDefuseIconID;
						break;

					case OBJECTIVE_ROLE_PLANT_EXPLOSIVES:
						iIconID = m_iPlantIconID;
						break;

					default:
						bDrawNumPeople = true;

				}

				if( iIconID != -1 )
				{
					pSurface->DrawSetTexture( iIconID );
					pSurface->DrawTexturedRect(x + (0.5 * iconWidth ), y + (0.5 * iconWidth ), x + iconWidth, y + iconHeight);
				}

				if( bDrawNumPeople )
				{	
					wchar_t cap[ 12 ];
					_snwprintf( cap, sizeof( cap ), L"" );

					
					if( pCaptureArea )
					{
						switch( pCapturePoint->GetCapturingTeamId() )
						{
							case TEAM_AXIS:
								if ( pCaptureArea->GetAxisCapCount() > 1 )
									_snwprintf( cap, sizeof( cap ), L"%i / %i", pCapturePoint->GetOccupantsByTeam( TEAM_AXIS ), pCaptureArea->GetAxisCapCount() );
								else
									_snwprintf( cap, sizeof( cap ), L"%i", pCapturePoint->GetOccupantsByTeam( TEAM_AXIS ) );
								break;

							case TEAM_CWEALTH:
								if ( pCaptureArea->GetAlliesCapCount() > 1 )
									_snwprintf( cap, sizeof( cap ), L"%i / %i", pCapturePoint->GetOccupantsByTeam( TEAM_CWEALTH ), pCaptureArea->GetAlliesCapCount() );
								else
									_snwprintf( cap, sizeof( cap ), L"%i", pCapturePoint->GetOccupantsByTeam( TEAM_CWEALTH ) );
								break;
						}
					}

					int wide, high;

					if( bLocalPlayerInArea )
					{
						pSurface->DrawSetTextFont( m_hTitleFont );
						pSurface->GetTextSize( m_hTitleFont, cap, wide, high );

						pSurface->DrawSetTextColor( Color( 255, 255, 255, 255 ) );
						pSurface->DrawSetTextPos( x + ( 180 * scaler ) + iconWidth - wide, y + 2 );
					}
					else
					{
						pSurface->DrawSetTextFont( m_hPlayerFont );
						pSurface->GetTextSize( m_hPlayerFont, cap, wide, high );

						Color capTeamColor = g_PR->GetTeamColor( pCapturePoint->GetCapturingTeamId() );
						pSurface->DrawSetTextColor( capTeamColor );
						pSurface->DrawSetTextPos( x + ( iconWidth / 2 ) - (wide / 2 ), y + iconHeight  );
					}

					pSurface->DrawPrintText( cap, wcslen(cap) );
				}

				if( bLocalPlayerInArea )
				{
					int ownerTeam = pCapturePoint->GetOwnerTeamId();
					C_Team *pTeam = GetGlobalTeam( ownerTeam );

					pSurface->DrawSetTextFont( m_hTitleFont );

					if( pTeam )
						pSurface->DrawSetTextColor( g_PR->GetTeamColor( ownerTeam ) );
					else
						pSurface->DrawSetTextColor( Color( 150, 150, 150, 255 ) );	

					pSurface->DrawSetTextPos( x + iconWidth + padding, y + 2 );

					wchar_t cap_name[ 120 ];
					_snwprintf( cap_name, sizeof( cap_name ), L"%S", pCapturePoint->GetNameOfZone() );

					pSurface->DrawPrintText( cap_name, wcslen(cap_name) );

					pSurface->DrawSetTextFont( m_hDetailFont );
					pSurface->DrawSetTextPos( x + iconWidth + padding, y + ( 14 * scaler ) );
					pSurface->DrawSetTextColor( Color( 255, 255, 255, 255 ) );	

					wchar_t detail[ 250 ];
					const char* translation = pCapturePoint->GetStringForRole();

					if( translation != NULL )
					{
						vgui::localize()->ConstructString( detail, sizeof(detail), vgui::localize()->Find( translation ), 0 );
					}
					else
					{
						int count = 0;
						wchar_t players[250] = L"";

						for( int i = 0; i < MAX_PLAYERS; i++ )
						{
							EHANDLE hPlayer = pCapturePoint->m_playersInArea[i];

							if( hPlayer )
							{
								C_BasePlayer *pOccupier = dynamic_cast<C_BasePlayer*>((C_BaseEntity*)hPlayer);

								if( pOccupier )
								{
									if( pOccupier->GetTeamNumber() == pPlayer->GetTeamNumber() )
									{
										if( count == 0 )
											_snwprintf( players, sizeof( players ), L"%S", pOccupier->GetPlayerName() );
										else
											_snwprintf( players, sizeof( players ), L"%s, %S", players, pOccupier->GetPlayerName() );

										count++;
									}
								}
							}
						}

						int needed = 0;

						if( count == 0 ) // no players
						{
							vgui::localize()->ConstructString( detail, sizeof(detail), vgui::localize()->Find( "#HaJ_NoPlayers" ), 0 );
						}
						else
						{
							vgui::localize()->ConstructString( detail, sizeof(detail), vgui::localize()->Find( "#HaJ_CapturePlayers" ), 1, players );
						}
						

						if( pCaptureArea ) // only capture areas will have this
						{
							if( pPlayer->GetTeamNumber() == TEAM_AXIS )
							{
								needed = pCaptureArea->GetAxisCapCount();	
							}
							else if( pPlayer->GetTeamNumber() == TEAM_CWEALTH )
							{
								needed = pCaptureArea->GetAlliesCapCount();
							}

							if( needed - count > 0 )
							{
								wchar_t need[75];

								int needs = needed - count;

								wchar_t needschar[5];
								_snwprintf( needschar, sizeof( needschar), L"%d", needs );

								if( needs == 1 )
								{
									vgui::localize()->ConstructString( need, sizeof(need), vgui::localize()->Find( "#HaJ_CaptureNeedPlayer" ), 0 );		
								}
								else
								{
									vgui::localize()->ConstructString( need, sizeof(need), vgui::localize()->Find( "#HaJ_CaptureNeedPlayers" ), 1, needschar );	
								}

								_snwprintf( detail, sizeof( detail ), L"%s%s", detail, need );							
							}
						}
					}

					pSurface->DrawPrintText( detail, wcslen( detail ) );

					x += ( 180 * scaler );

				}
			}
		}

		// TODO: put this INSIDE the if block to stop it leaving a gap for HUD elements
		// that shouldnt show. It's only here for testing right now.
		x += iconWidth + padding;

		itObj = sortedObjectives.NextInorder(itObj);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::Reset()
{

}

/////////////////////////////////////////////////////////////////////////////
void CHudCapturePoint::VidInit()
{
	// get an ID for our padlock icon.
	m_iLockedIconID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iLockedIconID, "hud/obj_overlay_locked", false, true );

	// ID for shielf
	m_iDefendIconID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iDefendIconID, "hud/obj_overlay_defend", false, true );

	// ID for destroyed icon
	m_iDestroyedIconID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iDestroyedIconID, "hud/obj_overlay_destroyed", false, true );

	m_iPlantIconID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iPlantIconID, "hud/obj_overlay_destroy", false, true );

	m_iDefuseIconID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iDefuseIconID, "hud/obj_overlay_defuse", false, true );

	// icon for 
	m_pIconBackground = gHUD.GetIcon( "obj_background" );
	m_pAlertIcon = gHUD.GetIcon( "obj_flash" );
}

/////////////////////////////////////////////////////////////////////////////
