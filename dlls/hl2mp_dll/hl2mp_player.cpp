//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "haj_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "datacache/imdlcache.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

#include "haj_cvars.h"
#include "haj_team.h"
#include "haj_viewvectors.h"
#include "haj_gamerules.h"
#include "haj_mapsettings_enums.h"
#include "haj_weapon_base.h"

#include "concuss.h"
#include "util.h"

// *HAJ 020* - Jed
int g_iLastCWealthModel = 0;
int g_iLastAxisModel = 0;

CBaseEntity	 *g_pLastAxisSpawn = NULL;
CBaseEntity	 *g_pLastCWealthSpawn = NULL;

extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE	0.3
#define HAJ_DAMAGEFALLOUT		0.01

void ClientKill( edict_t *pEdict );
void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

//LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropAngle( SENDINFO_VECTORELEM( m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM( m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
	SendPropInt( SENDINFO( m_iCurrentClass ), 3 ), // *HAJ 020* - Jed Rank variable	
	SendPropInt( SENDINFO( m_iRank ), 4 ), // *HAJ 020* - Jed Rank variable
	SendPropInt( SENDINFO( m_iDesiredClass ), 5 ),
	SendPropInt( SENDINFO( m_iDesiredTeam ), 6),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

// *HAJ 020* - Jed
// Player model lists.
extern const char *g_ppszBritishModels[CLASS_LAST];
extern const char *g_ppszCanadianModels[CLASS_LAST];
extern const char *g_ppszPolishModels[CLASS_LAST];
extern const char *g_ppszGermanModels[CLASS_LAST];

// END HAJ

#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() /*: m_PlayerAnimState( this )*/
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	m_flNextClassChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	BaseClass::ChangeTeam( TEAM_UNASSIGNED );

	// *HaJ 020* - SteveUK
	// Do the initial values for the desired class and team malark
	m_iDesiredClass = 0;
	m_iDesiredTeam = 0;

//	UseClientSideAnimation();

	// *HAJ 020* - Jed
	// Class related stuff
	m_iClass		= CLASS_UNASSIGNED;
	m_iCurrentClass = m_iClass;

	m_bMenuOpen = true;
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

//-----------------------------------------------------------------------------
// HAJ 020
// Purpose: Adds view offset to existing EyeAngles
// Client only
// B0B 5/9/2006
// Fixed BOB's code... works good now  - Pie
//-----------------------------------------------------------------------------
void CHL2MP_Player::AddViewOffset( const QAngle &angleOffset )
{
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: If you have chosen a class (current class is no unassigned)
//-----------------------------------------------------------------------------
bool CHL2MP_Player::HasChosenClass()
{
	return ( GetCurrentClass() == CLASS_UNASSIGNED ) ? false : true;
}
//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: If your on a valid team
//-----------------------------------------------------------------------------
bool CHL2MP_Player::IsValidTeam( bool bIncludeSpectator )
{
	if ( bIncludeSpectator )
		return (GetTeamNumber() > TEAM_UNASSIGNED && GetTeamNumber() < TEAM_LAST );
	else 
		return (GetTeamNumber() > TEAM_SPECTATOR && GetTeamNumber() < TEAM_LAST );
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: If you are a valid class
//-----------------------------------------------------------------------------
bool CHL2MP_Player::IsValidClass( int c )
{
	return ( c > CLASS_UNASSIGNED && c < CLASS_LAST );
}


//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Set the player class
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetPlayerClass()
{
	DevMsg( "Attempting to set %s's class\n", GetPlayerName() );

	if ( !IsObserver() && !g_fGameOver )
	{
		switch( m_iClass )
		{
		case CLASS_RIFLEMAN:
			SetClassRifleman();
			break;
		
		case CLASS_ASSAULT:
			SetClassAssault();
			break;
		
		case CLASS_SUPPORT:
			SetClassSupport();
			break;
		
		case CLASS_UNASSIGNED:
		default:
			break;
		}
	}

	// set current class to chose class
	m_iCurrentClass = m_iClass;
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Set the player model
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetPlayerModel( int pTeam, int pClass )
{
	// Try and grab our game rules.
	CHajGameRules *pGameRules = HajGameRules();
	
	if( IsObserver() )
		return;

	// default fallover
	if( !pGameRules )
	{
		if ( pTeam == TEAM_CWEALTH )
			SetModel( g_ppszBritishModels[pClass] );
		else
			SetModel( g_ppszGermanModels[pClass] );
	}

	// Commonwealth
	else
	{
		SetModel( pGameRules->GetPlayerModel( GetTeamNumber(), pClass ) );

		DevMsg( "Getting team model for player %s returns path %s\n", GetPlayerName(),  pGameRules->GetPlayerModel( GetTeamNumber(), pClass ) );

		/*
		if ( pTeam == TEAM_CWEALTH )
		{
			switch ( pGameRules->GetCommonwealthNation() )
			{
				default:
				case NATION_CWEALTH_BRITAIN:
					SetModel( g_ppszBritishModels[pClass] );
					break;
			
				case NATION_CWEALTH_CANADA:
					SetModel( g_ppszCanadianModels[pClass] );
					break;
				
				case NATION_CWEALTH_POLAND:
					SetModel( g_ppszPolishModels[pClass] );
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
					SetModel( g_ppszGermanModels[pClass] );
					break;
			}
		}
		*/
	}
}


//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Post class change stuff
//-----------------------------------------------------------------------------
void CHL2MP_Player::OnClassChange()
{
	if (m_iCurrentClass != m_iClass )
	{	
		m_iCurrentClass = m_iClass;
	}
	
	// we need to add something here to handle changing class!

}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Set Rifleman Class
//-----------------------------------------------------------------------------
extern ConVar haj_rifleman_grenades;
void CHL2MP_Player::SetClassRifleman()
{
	// Ditch everything we're carrying
	// Note: Ditch suit too?
	RemoveAllItems( false ); // true includes the HEV suit.

	DevMsg( "Spawning %s rifleman kit\n", GetPlayerName() );

	// Set weapons based on side.
	// Note: Eventually we should base this on country rather than team.
	int i = GetTeamNumber();

	SetPlayerModel( i, CWEALTH_DEFAULT_RIFLEMAN );
				
	switch ( i )
	{
	case TEAM_CWEALTH:
		GiveNamedItem( "weapon_enfield" );

		if( haj_rifleman_grenades.GetBool() )
			GiveNamedItem( "weapon_millsbomb" );

		GiveNamedItem( "weapon_fsknife" );
		
		if( haj_rifleman_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_rifleman_grenades.GetInt()-1,	"mills_grenade" );

		Weapon_Switch( Weapon_OwnsThisType( "weapon_enfield" ) );

		break;

	case TEAM_AXIS:
		DevMsg("Class: Axis/Rifleman\n");
		GiveNamedItem( "weapon_kar98" );	

		if( haj_rifleman_grenades.GetBool() )
			GiveNamedItem( "weapon_stielgranate" );

		GiveNamedItem( "weapon_shovel" );
	
		if( haj_rifleman_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_rifleman_grenades.GetInt()-1, "stick_grenade" );
	
		Weapon_Switch( Weapon_OwnsThisType( "weapon_kar98" ) );
		break;

	// YOU LOSE! YOU GET NOTHING! GOODAY SIR!
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Set Assualt Class
//-----------------------------------------------------------------------------
extern ConVar haj_assault_grenades;
void CHL2MP_Player::SetClassAssault()
{
	// Ditch everything we're carrying
	// Note: Ditch suit too?
	RemoveAllItems( false ); // true includes the HEV suit.

	DevMsg( "Spawning %s with assault kit\n", GetPlayerName() );

	// Set weapons based on side.
	// Note: Eventually we should base this on country rather than team.
	int i = GetTeamNumber();

	SetPlayerModel( i, CWEALTH_DEFAULT_ASSAULT );
	CHajGameRules *pGamerules = HajGameRules();

	switch ( i )
	{
	case TEAM_CWEALTH:

		if( pGamerules->GetYear() < YEAR_1941 || pGamerules->GetCommonwealthUnit() == UNIT_CWEALTH_COMMANDO )
			GiveNamedItem( "weapon_thompson" );
		else if( pGamerules->GetYear() > YEAR_1943 && pGamerules->GetCommonwealthUnit() == UNIT_CWEALTH_AIRBORNE && pGamerules->GetCommonwealthNation() == NATION_CWEALTH_BRITAIN )
			GiveNamedItem( "weapon_sten5" ); // mkV
		else
			GiveNamedItem( "weapon_sten" ); // mkII

		if( haj_assault_grenades.GetBool() )
			GiveNamedItem( "weapon_millsbomb" );

		GiveNamedItem( "weapon_no77smoke" );
		GiveNamedItem( "weapon_fsknife" );

		//CBasePlayer::GiveAmmo( 255,	"9mm" );
		if( haj_assault_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_assault_grenades.GetInt()-1,	"mills_grenade" );
		
		Weapon_Switch( Weapon_OwnsThisType( "weapon_thompson" ) );
		Weapon_Switch( Weapon_OwnsThisType( "weapon_sten" ) );
		Weapon_Switch( Weapon_OwnsThisType( "weapon_sten5" ) );
		break;

	case TEAM_AXIS:
		GiveNamedItem( "weapon_mp40" );

		if( haj_assault_grenades.GetBool() )
			GiveNamedItem( "weapon_stielgranate" );

		GiveNamedItem( "weapon_nb39smoke" );
		GiveNamedItem( "weapon_shovel" );

		//CBasePlayer::GiveAmmo( 255,	"9mm" );
		if( haj_assault_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_assault_grenades.GetInt()-1,	"stick_grenade" );

		Weapon_Switch( Weapon_OwnsThisType( "weapon_mp40" ) );
		break;

	// YOU LOSE! YOU GET NOTHING! GOODAY SIR!
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Set Support Class
//-----------------------------------------------------------------------------
extern ConVar haj_support_grenades;
void CHL2MP_Player::SetClassSupport()
{
	DevMsg( "Spawning %s support kit\n", GetPlayerName() );

	// Ditch everything we're carrying
	// Note: Ditch suit too?
	RemoveAllItems( false ); // true includes the HEV suit.

	// Set weapons based on side.
	// Note: Eventually we should base this on country rather than team.
	int i = GetTeamNumber();

	SetPlayerModel( i, CWEALTH_DEFAULT_SUPPORT );
	CHajGameRules *pGamerules = HajGameRules();

	switch ( i )
	{
	case TEAM_CWEALTH:

		if( pGamerules->GetYear() <= YEAR_1941 )
			GiveNamedItem( "weapon_bren"); // mk1
		else
			GiveNamedItem( "weapon_bren2" );

		if( haj_support_grenades.GetBool() )
			GiveNamedItem( "weapon_millsbomb" );

		GiveNamedItem( "weapon_fsknife" );

		if( haj_support_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_support_grenades.GetInt()-1,	"mills_grenade" );
		
		Weapon_Switch( Weapon_OwnsThisType( "weapon_bren" ) );
		Weapon_Switch( Weapon_OwnsThisType( "weapon_bren2" ) );
		break;

	case TEAM_AXIS:
		GiveNamedItem( "weapon_mg34" );

		if( haj_support_grenades.GetBool() )
			GiveNamedItem( "weapon_stielgranate" );

		GiveNamedItem( "weapon_shovel" );

		if( haj_support_grenades.GetInt() > 1 )
			CBasePlayer::GiveAmmo( haj_support_grenades.GetInt()-1,	"stick_grenade" );

		Weapon_Switch( Weapon_OwnsThisType( "weapon_mg34" ) );
		break;

	// YOU LOSE! YOU GET NOTHING! GOODAY SIR!
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Change class
//-----------------------------------------------------------------------------
void CHL2MP_Player::ChangeClass( int iNewClass )
{
	if ( m_iCurrentClass != iNewClass)
	{		
		m_iClass = iNewClass;
	}
}
//-----------------------------------------------------------------------------
// *HAJ 020* - Ztormi
// Purpose: Check if classlimit is not full
//-----------------------------------------------------------------------------
bool CHL2MP_Player::IsClassFull( int iClass )
{
	//Class limits - Ztormi
	int iNumClass = 0;

	// CHECK FOR DESIRED TEAM!
	int iTeam = GetTeamNumber();

	if( GetDesiredTeam() != 0 )
		iTeam = GetDesiredTeam();

	CHajGameRules* pGameRules = HajGameRules();

	if (!pGameRules)
		return false;

	int iLimit = pGameRules->GetClasslimitForClass( iClass, iTeam );

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

			if (!pPlayer)
				continue;

			CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

			if (!pHL2Player)
				continue;	
			
			if (pHL2Player->GetTeamNumber() == iTeam ) // Let's just count the classes on our own team
			{
				if ( pHL2Player->GetCurrentClass() == iClass || pHL2Player->GetDesiredClass() == iClass ) // Count the riflemen
					iNumClass++;
			}
	}
	
	// If the limit is set to -1 there's no limit
	if ( iLimit < 0 )
	{	
		DevMsg( "Class limit for %d is %d\n", iClass, pGameRules->GetClasslimitForClass( iClass, iTeam ) );
		return false;
	}

	DevMsg("Number of players: %d\nClass Limit: %d\n", iNumClass, iLimit );

	if( iNumClass >= iLimit )
	{
		if ( iClass == CLASS_RIFLEMAN ) // If there are too many assaulters we can't join
			ClientPrint( this, HUD_PRINTTALK, "#HaJ_Class_RifleFull" );
		else if ( iClass == CLASS_ASSAULT ) // If there are too many assaulters we can't join
			ClientPrint( this, HUD_PRINTTALK, "#HaJ_Class_AssaultFull" );
		else if ( iClass == CLASS_SUPPORT )// If there are too many machinegunners we can't join
			ClientPrint( this, HUD_PRINTTALK, "#HaJ_Class_MgFull" );

		if( GetTeamNumber() == TEAM_SPECTATOR || GetTeamNumber() == TEAM_UNASSIGNED )
		engine->ClientCommand( edict(), "chooseclass" );

		return true;
	}

	return false;

}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	// *HAJ 020* - Jed
	// Cache only the models we need.

	// Try and grab our game rules.
	CHajGameRules *pGameRules = HajGameRules();
	
	if( !pGameRules )
	{
		//Precache British/German models
		for ( int i = 0; i < ARRAYSIZE( g_ppszBritishModels ); ++i )
	   		PrecacheModel( g_ppszBritishModels[i] );

		for ( int i = 0; i < ARRAYSIZE( g_ppszGermanModels ); ++i )
			PrecacheModel( g_ppszGermanModels[i] );
	}
	else
	{
		// Commonwealth
		switch ( pGameRules->GetCommonwealthNation() )
		{
			default:
			case NATION_CWEALTH_BRITAIN:
				for ( int i = 0; i < ARRAYSIZE( g_ppszBritishModels ); ++i )
					PrecacheModel( g_ppszBritishModels[i] );
				break;
			
			case NATION_CWEALTH_CANADA:
				for ( int i = 0; i < ARRAYSIZE( g_ppszCanadianModels ); ++i )
					PrecacheModel( g_ppszCanadianModels[i] );
				break;
			
			case NATION_CWEALTH_POLAND:
				for ( int i = 0; i < ARRAYSIZE( g_ppszPolishModels ); ++i )
					PrecacheModel( g_ppszPolishModels[i] );
				break;
		};

		// Axis
		switch ( pGameRules->GetAxisNation() )
		{
			default:
			case NATION_AXIS_GERMANY:
				for ( int i = 0; i < ARRAYSIZE( g_ppszGermanModels ); ++i )
					PrecacheModel( g_ppszGermanModels[i] );
				break;
		};
	}
	// END HAJ
	
	PrecacheFootStepSounds();

	// precache the death sounds
	PrecacheScriptSound( "Commonwealth.die" );
	PrecacheScriptSound( "Axis.die" );
}

void CHL2MP_Player::GiveAllItems( void )
{
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	// *HAJ 020* - Jed
	// This is HAJ so you get NOTHING by default;
	EquipSuit( false );
	return;
}

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	// Handle Unassigned
	if ( GetTeamNumber() == TEAM_UNASSIGNED )
	{
		if ( HL2MPRules()->IsTeamplay() == false )
        {
            if ( GetModelPtr() == NULL )
            {
                ChangeTeam( TEAM_UNASSIGNED );
            }
        }
        else
        {
            ChangeTeam( TEAM_SPECTATOR );
        }
	}
}

int CHL2MP_Player::GetRank( void )
{
	// will need to update if there's different amounts of ranks on each side
	int iCE = FragCount() + 100;

	if( iCE >= 160 )
		return CWEALTH_RANK_SSGT;
	else if( iCE >= 140 )
		return CWEALTH_RANK_SGT;
	else if( iCE >= 120 )
		return CWEALTH_RANK_CPL;
	else if( iCE >= 105 )
		return CWEALTH_RANK_LCPL;
	
	return CWEALTH_RANK_PVT; // private by default
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{

	/*
	// JED - This was cassing massive asserts, I think the way I spawn bots
	if( IsBot() == true ) // some attempted bot fixes.
	{
		RemoveAllItems( true );
		State_Transition( STATE_OBSERVER_MODE );
		RemoveFlag( FL_FROZEN );
		StopObserverMode();
	}
	*/

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	m_flNextClassChangeTime = 0.0f;

	PickDefaultSpawnTeam();
	BaseClass::Spawn();

	//m_lifeState = LIFE_DEAD;
	//pl.deadflag = true;
	
	if ( !IsObserver() )
	{
		m_lifeState = LIFE_ALIVE;
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		
		RemoveEffects( EF_NODRAW );
		
		GiveDefaultItems();

		m_iRank = GetRank();
		
		// set rank badge on player model
		SetBodygroup( PLAYER_BODYGROUP_RANK, m_iRank );
		
		AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	}
	else
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
		SetGravity( 0.0f );
	}

	RemoveEffects( EF_NOINTERP );

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;
	
	m_Local.m_iHideHUD = 0;
	
	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		StartObserverMode( OBS_MODE_FIXED );

		ShowViewPortPanel( "specmenu", false );
		ShowViewPortPanel( "specgui", false );
		ShowViewPortPanel( "overview", false );

		AddFlag( FL_FROZEN );
		AddEffects( EF_NODRAW );
		RemoveAllItems( true );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;
	m_Local.m_bProned = false;	// *HaJ 020* - Jed

	SetPlayerUnderwater(false);

	m_bReady = false;

	if ( GetTeamNumber() != TEAM_SPECTATOR )
		StopObserverMode();
	else if( GetObserverMode() != OBS_MODE_FIXED )
	    StartObserverMode( OBS_MODE_ROAMING );
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

bool CHL2MP_Player::ValidatePlayerModel( const char *pModel )
{
	// *HAJ 020* - Jed
	// HaJ Models
	int iModels = ARRAYSIZE( g_ppszBritishModels );
	int i;	

	for ( i = 0; i < iModels; ++i )
	{
		if ( !Q_stricmp( g_ppszBritishModels[i], pModel ) )
		{
			return true;
		}
	}

	iModels = ARRAYSIZE( g_ppszGermanModels );

	for ( i = 0; i < iModels; ++i )
	{
	   	if ( !Q_stricmp( g_ppszGermanModels[i], pModel ) )
		{
			return true;
		}
	}
	// END HAJ

	return false;
}

void CHL2MP_Player::SetPlayerTeamModel( void )
{
	const char *szModelName = NULL;
	// *HAJ 020* - Jed
	// I'm not setting the player model based on the console variable,
	// I do it by team number
	/*
	//szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	int modelIndex = modelinfo->GetModelIndex( szModelName );

	// Default player model, I think (Jed)
	if ( modelIndex == -1 || ValidatePlayerModel( szModelName ) == false )
	{
		szModelName = "models/player.mdl";
		m_iModelType = TEAM_UNASSIGNED;

		char szReturnString[512];

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}
	*/
	
	// *HAJ 020* - Jed
	// Change the default check for the player name - we're just setting it no matter what.
	
	switch ( GetTeamNumber() )
	{
	
	case TEAM_AXIS:
		szModelName = g_ppszGermanModels[0];
		m_iModelType = TEAM_AXIS;
		break;

	case TEAM_CWEALTH:
		szModelName = g_ppszBritishModels[0];
		m_iModelType = TEAM_CWEALTH;
		break;
	
	case TEAM_UNASSIGNED:
	default:
		szModelName = g_ppszBritishModels[0];
		m_iModelType = TEAM_CWEALTH;
		break;
	}

	SetModel( szModelName );
	SetupPlayerSoundsByModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	// *HAJ 020* - Jed 
	// We're only human after all
	m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		MDLCACHE_CRITICAL_SECTION();

		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	//m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

	// *HAJ 020* - Jed
	// Check if the class has changed
	OnClassChange();
}

void CHL2MP_Player::PlayerDeathThink()
{
	BaseClass::PlayerDeathThink();
}


void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_iDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	// *HAJ 020* - Jed - record time last shot was fired.
	//SetLastShotTime( gpGlobals->curtime );

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_AXIS || m_iModelType == TEAM_CWEALTH )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{

}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( IsEFlagSet( EFL_NO_WEAPON_PICKUP ) )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
	/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	// *HaJ 020* - SteveUK
	// This is a little reworked with the desired team stuff.

	if( iTeam == TEAM_SPECTATOR ) // we're either going to or from spectate.
	{
		BaseClass::ChangeTeam( iTeam );
		m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;
		SetPlayerTeamModel();

		RemoveAllItems( true ); // so we can't kill some mo-fos in spec
		State_Transition( STATE_OBSERVER_MODE ); // GUI
	}
	else
	{
		if( GetTeamNumber() == TEAM_SPECTATOR )
		{
			StopObserverMode();
			State_Transition( STATE_ACTIVE );

			m_lifeState = LIFE_DEAD;
		}
		/*else
		{
			ClientKill( edict() );
		}*/

		BaseClass::ChangeTeam( iTeam );
		StartObserverMode( OBS_MODE_FIXED );
	}

}

//-----------------------------------------------------------------------------
// Purpose: Handles jointeam command
//-----------------------------------------------------------------------------
bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	// *HAJ 020*
	// don't swap if your already on this team
	if ( GetTeamNumber() == team )
		return true;

	// Auto-assign
	if ( team == TEAM_AUTO )	// Auto assign
	{
		CTeam *pAxis = g_Teams[TEAM_AXIS];
		CTeam *pCWealth = g_Teams[TEAM_CWEALTH];

		if ( pAxis == NULL || pCWealth == NULL )
		{
			team = random->RandomInt( TEAM_CWEALTH, TEAM_AXIS );
		}
		else
		{
			if ( pAxis->GetNumPlayers() > pCWealth->GetNumPlayers() )
			{
				team = TEAM_CWEALTH;
			}
			else if ( pAxis->GetNumPlayers() < pCWealth->GetNumPlayers() )
			{
				team = TEAM_AXIS;
			}
			else // consider team score.
			{
				if( pAxis->GetScore() > pCWealth->GetScore() )
					team = TEAM_CWEALTH;
				else if( pCWealth->GetScore() > pAxis->GetScore() )
					team = TEAM_AXIS;
				else
					team = random->RandomInt( TEAM_CWEALTH, TEAM_AXIS );
			}
		}
	}

	// *HaJ 020* - SteveUK
	// This is implementing the desired team stuff here.
	if( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_SPECTATOR && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
			ClientKill( edict() );
			
			m_iClass = CLASS_UNASSIGNED;
			m_iCurrentClass = m_iClass;
		}
		
		ChangeTeam( TEAM_SPECTATOR );
		return true;		
	}

	// we're swapping team so clear our current class;
	m_iClass = CLASS_UNASSIGNED;
	m_iCurrentClass = m_iClass;
	SetDesiredClass( CLASS_UNASSIGNED );
	m_flNextClassChangeTime = gpGlobals->curtime;

	ChangeTeam( team );

	return true;
}

int CHL2MP_Player::GetActiveClass( )
{
	return m_iClass;
}

//-----------------------------------------------------------------------------
// Purpose: Handles JoinClass command
//-----------------------------------------------------------------------------
bool CHL2MP_Player::HandleCommand_JoinClass( int c )
{
	if ( !IsValidClass( c ) )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", c );
		return false;
	}

	// *HAJ 020* Ztormi
	// check if the class is full
	if ( IsClassFull( c ) )
		return false;

	if( gpGlobals->curtime < m_flNextClassChangeTime )
	{
		// Get the time from now they're allowed to switch
		int switchtime = (int)(m_flNextClassChangeTime - gpGlobals->curtime);
		
		// Print it into a char variable so we can print it on the screen
		char SwitchText[5];
		Q_snprintf( SwitchText, sizeof( SwitchText ), "%d", switchtime );
		
		// Print it on the screen.
		if( switchtime <= 1 )
		{
			ClientPrint( this, HUD_PRINTTALK, "#HaJ_Class_ChangeOneSec" );
		}
		else
		{
			ClientPrint( this, HUD_PRINTTALK, "#HaJ_Class_ChangeIn", SwitchText );
		}
		
		return false;
	}
	else
	{
		m_flNextClassChangeTime = gpGlobals->curtime + haj_classchangefreq.GetInt();
	}

	// *HAJ 020*
	// don't swap if your already this class
	if ( c == GetCurrentClass() )
		return true;

	// ok, we're storing the class as desired, we'll actually switch when we die
	SetDesiredClass( c );

	CHajGameRules *pGamerules = HajGameRules();

	// respawn us if we're unassigned and flagged for being dead.
	if( ( GetCurrentClass() == CLASS_UNASSIGNED && !pGamerules->IsSuddenDeath() && !pGamerules->IsRoundBased() ) || (pGamerules->IsFreezeTime() || pGamerules->IsFreeplay()) )
	{
		CheckDesired();
		Spawn();
	}

	// quick dirty cheap way to get the localization thing
	char Localization[50];
	//Q_snprintf( Localization, sizeof( Localization ), "#HaJ_Class_%d", c );

	// tell them what we've picked!
	//UTIL_ClientPrintAll( HUD_PRINTTALK, Localization, this->GetPlayerName() ); 

	// now lets do another for the center of the screen to tell the actual player about the class they've selected!
	Q_snprintf( Localization, sizeof( Localization ), "#HaJ_Class_%d", c );
	ClientPrint( this, HUD_PRINTTALK, "#HaJ_Local_ChangeClass", Localization );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles client commands
//-----------------------------------------------------------------------------
bool CHL2MP_Player::ClientCommand( const char *pcmd )
{
	if ( FStrEq( pcmd, "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( pcmd ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( pcmd, "jointeam" ) ) 
	{
		if ( engine->Cmd_Argc() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( pcmd ) )
		{
			int iTeam = atoi( engine->Cmd_Argv(1) );

			if ( GetTeamNumber() != iTeam )
				HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "joingame" ) )
	{
		return true;
	}
	
	// *HAJ 020* - Jed
	// Change class
	else if ( FStrEq( pcmd, "joinclass" ) ) 
	{
		if ( engine->Cmd_Argc() < 2 )
		{
			Warning( "Player sent bad joinclass syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( pcmd ) )
		{
			int c = atoi( engine->Cmd_Argv(1) );

			if ( c != GetCurrentClass() )
				HandleCommand_JoinClass( c );
		}
		return true;
	}

	else if ( FStrEq( pcmd, "menuclosed" ) )
	{
		m_bMenuOpen = false;
		return true;
	}

	else if ( FStrEq( pcmd, "menuclosed" ) )
	{
		m_bMenuOpen = true;
		return true;
	}
	
	return BaseClass::ClientCommand( pcmd );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	// *HAJ 020* - Jed
	// Spectators can't use impulse commands
	if (GetTeamNumber() == TEAM_SPECTATOR)
		return;

	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const char *pcmd )
{
	int i = m_RateLimitLastCommandTimes.Find( pcmd );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( pcmd, gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	// *HAJ 020*
	//	TODO: Revisit this and look at making it drop the right type of grenade
	// Drop a grenade if it's primed.
	/*
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}
	*/

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

/* HAJ 020 - Jed */
void CHL2MP_Player::Event_Dying()
{
	// stop concussion
	UTIL_Concuss( this, 0, CONCUSS_STOP);

	BaseClass::Event_Dying();
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	// stop concussion
	UTIL_Concuss( this, 0, CONCUSS_STOP);

	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.

	// *HAJ 020* - Jed
	// Don't draw ragdoll when you kill a spectator!
	if ( GetTeamNumber() != TEAM_SPECTATOR )
	{
		// if we got hit by a blast that was < 150 or killed some other way, ragdoll
		// we dont want the ragdoll otherwise as we'll gib.
		if ( !( info.GetDamageType() & DMG_BLAST ) || ( (info.GetDamageType() & DMG_BLAST) && info.GetDamage() < 150 ) )
		{
			CreateRagdollEntity();
		}
	}

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}

		GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

// *HaJ 020* - SteveUK
// This function returns the "desired" class of the player
int CHL2MP_Player::GetDesiredClass()
{
	return m_iDesiredClass;	
}

// *HaJ 020* - SteveUK
// As above, but with team
int CHL2MP_Player::GetDesiredTeam()
{
	return m_iDesiredTeam;	
}

// *HaJ 020* - SteveUK
// This function sets the "desired" class of the player
void CHL2MP_Player::SetDesiredClass( int iDesiredClass )
{
	m_iDesiredClass = iDesiredClass;	
}

// *HaJ 020* - SteveUK
// As above, but with team
void CHL2MP_Player::SetDesiredTeam( int iDesiredTeam )
{
	m_iDesiredTeam = iDesiredTeam;	
}

// *HaJ 020* - SteveUK
// This function checks if you have chosen a new team or class and does the actions required
void CHL2MP_Player::CheckDesired()
{
	if( GetDesiredClass() != 0 )
	{
		// Set it up!
		m_iClass = GetDesiredClass();
		m_iCurrentClass = m_iClass;

		SetDesiredClass( 0 ); // reset
	}

}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	return BaseClass::OnTakeDamage( inputInfo );
}

/* HAJ 020 - Jed */
// Do concussion if near an explosion
void CHL2MP_Player::DamageEffect( float flDamage, int fDamageType )
{
	if ( fDamageType & DMG_BLAST && IsAlive() )
	{	
		UTIL_Concuss( this, flDamage, CONCUSS_START);
	}
	
	BaseClass::DamageEffect( flDamage, fDamageType );
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	// *HAJ 020* - Jed
	// Dieing spectators don't scream.
	int i = GetTeamNumber();
	
	if ( i == TEAM_SPECTATOR)
		return;
		
	char szStepSound[128];

	// have it play the right death sound for the relevant team
	switch ( i )
	{
		case TEAM_AXIS:
			Q_strcpy( szStepSound, "Axis.Die");
			break;
		
		case TEAM_CWEALTH:
		default:
			Q_strcpy( szStepSound, "Commonwealth.Die");
			break;
	}

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity* pSpawnPointEnt = NULL;

	CHajTeam* pTeam = dynamic_cast<CHajTeam*>(GetTeam());
	if(pTeam && !g_fGameOver )
	{
		pSpawnPointEnt = pTeam->GetNextSpawnPoint(this);
	}
	else
	{
		// we shouldnt ever get here, but just in case we do
		// lets fallback to the default spawn point
		pSpawnPointEnt = gEntList.FindEntityByClassname( NULL, "info_player_start" );
	}

	return pSpawnPointEnt;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

//	const char *pReadyCheck = p;
//	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	//if ( m_bEnterObserver == true )
	//{
		return BaseClass::StartObserverMode( mode );
	//}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() && GetTeamNumber() != TEAM_SPECTATOR )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	/*
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
	*/
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}
