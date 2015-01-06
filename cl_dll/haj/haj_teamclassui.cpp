#include "cbase.h"

#include "haj_teamclassui.h"
#include "haj_player_c.h"
#include "haj_gamerules.h"
#include "c_playerresource.h"
#include "weapon_parse.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/BitmapImagePanel.h>

#include <cl_dll/iviewport.h>

#include "haj_modelpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CHAJClassSelection::CHAJClassSelection( IViewPort *pViewPort ) : BaseClass(NULL, PANEL_HAJCLASS )
{
	m_iCurTeam = -1;
	m_iSelectedClass = 0;
	m_iTabbedToClass = 0;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable( false );
	SetSizeable( false );

	SetProportional( true );
	SetTitleBarVisible( false );

	m_pBackground = new vgui::CBitmapImagePanel( this, "BackgroundImage", "gfx/vgui/menu_background" );

	m_pModelPanel = new CModelPanel( this, "PlayerModel" );
	m_pModelPanel->SetParent( this );

	m_pClassName = new vgui::Label( this, "ClassName", "" );
	m_pClassInfo = new vgui::Label( this, "ClassInfo", "Info about this class" );
	m_pClassTitle = new vgui::Label( this, "JoinClass", "#CM_ChooseClass" );
	m_pPlayerCount = new vgui::Label( this, "PlayerCount", "" );

	m_pCancelButton = new vgui::LocalImageButton( this, "CancelButton", "team/buttons/cancel", "cancelbutton", false );
	m_pConfirmButton = new vgui::LocalImageButton( this, "ConfirmButton", "team/buttons/confirm", "confirmselection", false );
	m_pSuicideOnClassChange = new vgui::CheckButton( this, "SuicideOnChange", "#HaJ_SuicideOnClassChange" );

	m_pClassBackground = new vgui::ImagePanel( this, "ClassBackground" );

	LoadControlSettings( "resource/UI/TeamClassMenu.res" );

	for( int i = CLASS_UNASSIGNED+1; i < CLASS_LAST; i++ )
	{
		// ugly Q_snprintf's for the shit
		char szPanelName[35];
		Q_snprintf( szPanelName, sizeof( szPanelName ), "ClassButton%d", i );

		char szCommand[35];
		Q_snprintf( szCommand, sizeof( szCommand ), "switchclasstab %d", i );

		char szClassName[35];
		Q_snprintf( szClassName, sizeof( szClassName ), "#HaJ_Class_%d", i );

		m_pClassTabs[i] = new vgui::Button( this, szPanelName, szClassName, this );
		m_pClassTabs[i]->SetPos( (GetWide() - m_iClassButtonsX - m_iClassButtonsW ) - ( ( m_iClassButtonsW + m_iClassButtonsSpacing ) * (CLASS_LAST - i - 1) ), m_iClassButtonsY );
		m_pClassTabs[i]->SetSize( m_iClassButtonsW, m_iClassButtonsH );

		m_pClassTabs[i]->SetVisible( true );
		m_pClassTabs[i]->SetEnabled( true );

		m_pClassTabs[i]->SetCommand( szCommand );
	}

	InvalidateLayout( );
}


void CHAJClassSelection::Think()
{

	//if( m_iSelectedClass > CLASS_UNASSIGNED )
	//	m_pClassTabs[m_iSelectedClass]->SetBgColor( m_SelectColor );

	C_HajPlayer *pLocalPlayer = C_HajPlayer::GetLocalHajPlayer();
	CHajGameRules *pGR = HajGameRules();

	if( pLocalPlayer && pGR && g_PR )
	{
		// count how many people have this class!
		int iClassCount = 0;
		int iClassLimit = pGR->GetClasslimitForClass( m_iSelectedClass, pLocalPlayer->GetTeamNumber() );

		for( int i = 1; i <= MAX_PLAYERS; i++ )
		{
			if( !g_PR->IsConnected( i ) )
				continue;

			if( pLocalPlayer->GetTeamNumber() == g_PR->GetTeam( i ) && g_PR->GetPlayerClass( i ) == m_iSelectedClass )
				iClassCount++;
		}

		wchar_t playerCountOut[75], pCount[20], classLimit[20];

		_snwprintf( pCount, sizeof( pCount ), L"%d", iClassCount );
		_snwprintf( classLimit, sizeof( classLimit ), L"%d", iClassLimit );

		// handle showing class limit
		if( iClassLimit >= 0 && iClassLimit < 255 )
			vgui::localize()->ConstructString( playerCountOut, sizeof( playerCountOut ), vgui::localize()->Find( "#CM_PlayerCountLimit" ), 2, pCount, classLimit );
		else
			vgui::localize()->ConstructString( playerCountOut, sizeof( playerCountOut ), vgui::localize()->Find( "#CM_PlayerCount" ), 1, pCount );

		if( ( iClassCount >= iClassLimit && iClassLimit >= 0 ) || m_iSelectedClass == CLASS_UNASSIGNED )
		{
			m_pConfirmButton->SetEnabled( false );
			m_pConfirmButton->SetAlpha( 100 );
		}
		else
		{
			m_pConfirmButton->SetEnabled( true );
			m_pConfirmButton->SetAlpha( 255 );
		}

		m_pPlayerCount->SetText( playerCountOut );
	}

}


void CHAJClassSelection::PaintBackground()
{
	if( m_Background )
		m_Background->DrawSelf( 0, 0, GetWide(), GetTall(), Color( 255, 255, 255 ) );
}

void CHAJClassSelection::ShowPanel( bool bShow )
{
	SetVisible( bShow );
	SetMouseInputEnabled( bShow );

	if( bShow )
	{
		MoveToCenterOfScreen();

		C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

		if( pPlayer )
		{
			int iClass = pPlayer->GetCurrentClass();

			if( pPlayer->GetDesiredClass() > CLASS_UNASSIGNED )
				iClass = pPlayer->GetDesiredClass();

			if( iClass == CLASS_UNASSIGNED )
				iClass = CLASS_RIFLEMAN;

			ChangeTab( iClass );
			m_iTabbedToClass = 0;
		}
	}
}

bool CHAJClassSelection::NeedsUpdate()
{
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();
	if( !pPlayer ) return false;

	return ( pPlayer->GetTeamNumber() != m_iCurTeam );
}

void CHAJClassSelection::Update()
{
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();
	if( !pPlayer ) return;

	SetupTeam( pPlayer->GetTeamNumber() );
}

void CHAJClassSelection::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_Background = gHUD.GetIcon( "menu_background" );

	m_pClassName->SetFont( m_ClassLabelFont );
	m_pClassInfo->SetFont( m_ClassDescriptionFont );
	m_pPlayerCount->SetFont( m_ClassDescriptionFont );
	m_pClassTitle->SetFont( m_TitleFont );

	m_pClassInfo->SetFgColor( m_DescriptionColor );
	m_pPlayerCount->SetFgColor( m_DescriptionColor );

	Color bgCol = Color(0,0,0,0);
	Color fgCol = m_pClassTabs[CLASS_RIFLEMAN]->GetFgColor();

	for( int i = CLASS_UNASSIGNED+1; i < CLASS_LAST; i++ )
	{
		m_pClassTabs[i]->SetFont( m_ClassTabFont );
		m_pClassTabs[i]->SetBorder( NULL );

		m_pClassTabs[i]->SetDefaultBorder(NULL);
		m_pClassTabs[i]->SetDepressedBorder(NULL);
		m_pClassTabs[i]->SetKeyFocusBorder(NULL);

		m_pClassTabs[i]->SetDefaultColor(fgCol,bgCol);
		m_pClassTabs[i]->SetArmedColor(fgCol,bgCol);
		m_pClassTabs[i]->SetDepressedColor(fgCol,bgCol);
	}
}

void CHAJClassSelection::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pBackground->SetSize( GetWide(), GetTall() );
	m_pBackground->SetPos( 0, 0 );

}

void CHAJClassSelection::ConfirmSelection( void )
{
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();
	if( !pPlayer ) return;

	char szJoinClassCommand[40];
	Q_snprintf( szJoinClassCommand, sizeof( szJoinClassCommand ), "joinclass %d", m_iSelectedClass );

	SetVisible( false );

	// we've got suicide on change selected, exec kill command.
	if( m_pSuicideOnClassChange->IsSelected() && pPlayer->IsAlive() && pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
	{
		engine->ClientCmd( "kill" );
	}

	engine->ClientCmd( szJoinClassCommand );
}

void CHAJClassSelection::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "cancelbutton" ) ) // we don't want to switch clases
	{
		SetVisible( false );
	}
	else if( !Q_stricmp( command, "confirmselection" ) ) // we do want to switch classes
	{
		ConfirmSelection();
	}
	else if( strncmp( command, "switchclasstab", 14 ) == 0 ) // change class tab
	{
		const char* num = command + 15; // +15 because of the space

		int iClass = atoi( num );
		ChangeTab( iClass );
	}

	BaseClass::OnCommand( command );
}

// Setup and load class info
void CHAJClassSelection::SetupTeam( int iTeam )
{
	m_iCurTeam = iTeam;	
	ChangeTab( m_iSelectedClass );
	m_iTabbedToClass = 0;
}

// Change the class info shown when navigating to a different tab
void CHAJClassSelection::ChangeTab( int iClass )
{
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();
	if( !pPlayer || !HajGameRules() ) return;

	/*if( m_iSelectedClass > CLASS_UNASSIGNED && m_pClassTabs[m_iSelectedClass] != NULL ) // fade old class
		m_pClassTabs[m_iSelectedClass]->SetBgColor( Color(0,0,0,0) );

	if( iClass > CLASS_UNASSIGNED && m_pClassTabs[iClass] != NULL )
		m_pClassTabs[iClass]->SetBgColor( m_SelectColor );*/

	if( iClass > CLASS_UNASSIGNED && m_iTabbedToClass > 0 && iClass == m_iTabbedToClass )
	{
		ConfirmSelection();
		m_iTabbedToClass = 0;

		return;
	}

	m_iTabbedToClass = iClass;
	
	char className[35];
	Q_snprintf( className, sizeof( className ), "#HaJ_Class_%d", iClass );

	m_pClassName->SetText( className );
	m_iSelectedClass = iClass;

	Q_snprintf( m_pModelPanel->m_ModelName, sizeof( m_pModelPanel->m_ModelName ), "%s", HajGameRules()->GetPlayerModel( pPlayer->GetTeamNumber(), (iClass > CLASS_UNASSIGNED) ? iClass : CLASS_RIFLEMAN ) );

	bool bChangedWeapon = false;

	if( iClass > CLASS_UNASSIGNED )
	{
		UpdateClassBackground( pPlayer->GetTeamNumber(), iClass );

		wchar_t szWeaponsp[250] = L"";
		wchar_t szItemsp[250] = L"";

		CUtlVector<inventoryItem_t*> items;
		HajGameRules()->GetPlayerLoadout( items, pPlayer->GetTeamNumber(), m_iSelectedClass );

		for( int i = 0; i < items.Count(); i++ )
		{
			inventoryItem_t* pItem = items.Element( i );

			if( pItem )
			{
				FileWeaponInfo_t *pWeaponInfo = GetFileWeaponInfoFromHandle( LookupWeaponInfoSlot( pItem->itemName ) );

				if( pWeaponInfo )
				{
					wchar_t szTemp[75] = L"";

					if( !bChangedWeapon )
					{
						// Steve - we need to do some sanity checks here, i.e. what ifthe animation does exist?
						// Updated to reflect the new anim names.
						Q_snprintf( m_pModelPanel->m_SequenceName, sizeof( m_pModelPanel->m_SequenceName ), "StandIdle_%s", pWeaponInfo->szAnimationPrefix );
						Q_snprintf( m_pModelPanel->m_WeaponModel, sizeof( m_pModelPanel->m_WeaponModel ), "%s", pWeaponInfo->szWorldModel );

						bChangedWeapon = true;
					}

					if( pItem->itemQuantity > -1 || pItem->itemType == ITEM_TYPE_LIMITED_ITEM )
					{
						int iAmmo = HajGameRules()->GetClampedStartingAmmoForClass( pItem->ammoIndex, pItem->itemQuantity, pPlayer->GetTeamNumber(), m_iSelectedClass );

						if( iAmmo > 0 )
						{
							_snwprintf( szTemp, sizeof( szTemp ), L"%dx %s\n\n", iAmmo, vgui::localize()->Find( pWeaponInfo->szPrintName ) );
							wcscat( szItemsp, szTemp );
						}
					}
					else
					{
						_snwprintf( szTemp, sizeof( szTemp ), L"%s\n\n", vgui::localize()->Find( pWeaponInfo->szPrintName ) );
						wcscat( szWeaponsp, szTemp );
					}
				}
			}
		}

		wchar_t szCombined[500];
		_snwprintf( szCombined, sizeof( szCombined ), L"%s\n\n%s", szWeaponsp, szItemsp );

		m_pClassInfo->SetText( szCombined );
	}
	else
	{
		m_pClassInfo->SetText( "#HaJ_SelectClassFromTab" );
		Q_snprintf( m_pModelPanel->m_SequenceName, sizeof( m_pModelPanel->m_SequenceName ), "idle_BOLT" );

		if( pPlayer->GetTeamNumber() == TEAM_AXIS )
			Q_snprintf( m_pModelPanel->m_WeaponModel, sizeof( m_pModelPanel->m_WeaponModel ), "models/weapons/w_models/w_k98.mdl" );
		else
			Q_snprintf( m_pModelPanel->m_WeaponModel, sizeof( m_pModelPanel->m_WeaponModel ), "models/weapons/w_models/w_enfield.mdl" );
	}

}

void CHAJClassSelection::UpdateClassBackground( int iTeam, int iClass )
{
	char szNewImage[64];
	Q_snprintf( szNewImage, sizeof( szNewImage ), "class/tab/tab_cw%d", iClass );

	m_pClassBackground->SetImage( szNewImage );
}