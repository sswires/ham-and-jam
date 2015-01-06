
// haj_gamerules.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"

#include "ammodef.h"
#include "haj_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "filesystem.h"
#include "haj_mapsettings_enums.h"
#include "haj_teaminfo.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "haj_player_c.h"
	#include "haj_bombzone_c.h"
#else
	#include "baseentity.h"
	#include "nav_mesh.h"
	#include "haj_player.h"
	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "iscorer.h"
	#include "hl2mp_player.h"
	#include "weapon_hl2mpbasehlmpcombatweapon.h"
	#include "team.h"
	#include "hl2mp_gameinterface.h"
	#include "hl2mp_cvars.h"
	#include "haj_cvars.h"
	#include "haj_team.h"
	#include "haj_mapentityfilter.h"
	#include "haj_gameinterface.h"
	#include "haj_roundtimeroverride.h"
	#include "haj_spawnpoint.h"
	#include "haj_bombzone.h"

	#include "haj_weapon_base.h"
	
	#ifdef DEBUG	
		#include "hl2mp_bot_temp.h"
	#endif
#endif

#include "haj_viewvectors.h"

#if GAME_DLL

ConVar mp_restartgame( "mp_restartgame", "0", 0, "If non-zero, game will restart in the specified number of seconds" );
ConVar sv_report_client_settings("sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );

#ifdef CLIENT_DLL
	ConVar cl_autowepswitch(
		"cl_autowepswitch",
		"1",
		FCVAR_ARCHIVE | FCVAR_USERINFO,
		"Automatically switch to picked up weapons (if more powerful)" );
#endif

extern ConVar mp_chattime;
extern ConVar haj_freezetime;

// *HAJ 020* - Jed
extern CBaseEntity	 *g_pLastAxisSpawn;
extern CBaseEntity	 *g_pLastCWealthSpawn;
#endif


// British
const char *g_ppszBritishModels[CLASS_LAST] = 
{
	"models/player/british/infantry_rifleman.mdl",
	"models/player/british/infantry_assault.mdl",
	"models/player/british/infantry_support.mdl",
	"", // CLASS_LAST
};

// Canadian
const char *g_ppszCanadianModels[CLASS_LAST] = 
{
	"models/player/canadian/infantry_rifleman.mdl",
	"models/player/canadian/infantry_assault.mdl",
	"models/player/canadian/infantry_support.mdl",
	"", // CLASS_LAST
};

// Polish
const char *g_ppszPolishModels[CLASS_LAST] = 
{
	"models/player/polish/infantry_rifleman.mdl",
	"models/player/polish/infantry_assault.mdl",
	"models/player/polish/infantry_support.mdl",
	"", // CLASS_LAST
};

// German
const char *g_ppszGermanModels[CLASS_LAST] =
{
	"models/player/german/infantry_rifleman.mdl",
	"models/player/german/infantry_assault.mdl",
	"models/player/german/infantry_support.mdl",
	"", // CLASS_LAST
};


/////////////////////////////////////////////////////////////////////////////
REGISTER_GAMERULES_CLASS( CHajGameRules );

/*

#ifdef CLIENT_DLL
// recieve the ammo array
RecvPropArray3( RECVINFO_ARRAY( m_iAmmoInClip ), RecvPropInt( RECVINFO( m_iAmmoInClip[0] )) ),

#else
// send the ammo array
SendPropArray3( SENDINFO_ARRAY3( m_iAmmoInClip ), SendPropInt( SENDINFO_ARRAY( m_iAmmoInClip ), 10, SPROP_UNSIGNED ) ),
#endif

*/

/////////////////////////////////////////////////////////////////////////////
BEGIN_NETWORK_TABLE_NOBASE( CHajGameRules, DT_HajGameRules )

#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_roundTimeLeft)),
	RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bWaitingForSpawn ), RecvPropBool( RECVINFO( m_bWaitingForSpawn[0] )) ),
	RecvPropArray3( RECVINFO_ARRAY( m_fSpawnTime ), RecvPropFloat( RECVINFO( m_fSpawnTime[0] )) ),
	RecvPropInt( RECVINFO( m_iGametype ) ),
	
	// map settings data
	RecvPropInt( RECVINFO( m_nCommonwealthNation ) ),
	RecvPropInt( RECVINFO( m_nAxisNation ) ),
	RecvPropInt( RECVINFO( m_nCommonwealthUnit ) ),
	RecvPropInt( RECVINFO( m_nAxisUnit ) ),
	RecvPropInt( RECVINFO( m_nCommonwealthInsignia ) ),
	RecvPropInt( RECVINFO( m_nAxisInsignia ) ),
	RecvPropInt( RECVINFO( m_nYear ) ),
	RecvPropInt( RECVINFO( m_nSeason ) ),
	RecvPropInt( RECVINFO( m_nTheatre ) ),

	RecvPropInt( RECVINFO( m_iRound ) ),
	RecvPropFloat( RECVINFO( m_flRoundStartTime ) ),
	RecvPropBool( RECVINFO( m_bOvertime ) ),
	RecvPropBool( RECVINFO( m_bSuddenDeath ) ),
	RecvPropBool( RECVINFO( m_bFreezeTime ) ),

	//Classlimit vars - Ztormi
	RecvPropInt( RECVINFO( m_iCommonwealthMachinegunnerlimit ) ),
	RecvPropInt( RECVINFO( m_iCommonwealthAssaultlimit ) ),
	RecvPropInt( RECVINFO( m_iCommonwealthRiflelimit ) ),
	RecvPropInt( RECVINFO( m_iWehrmachtMachinegunnerlimit ) ),
	RecvPropInt( RECVINFO( m_iWehrmachtAssaultlimit ) ),
	RecvPropInt( RECVINFO( m_iWehrmachtRiflelimit ) ),
	//---end Ztormi

	RecvPropInt( RECVINFO( m_iDefendingTeam ) ),
	RecvPropInt( RECVINFO( m_iAttackingTeam ) ),
		
	RecvPropBool( RECVINFO( m_bFreeplay ) ),
	RecvPropBool( RECVINFO( m_bOnEndGameScreen ) ),

	RecvPropEHandle( RECVINFO(m_GamemodeHandle ) ),
#else
	SendPropFloat(SENDINFO(m_roundTimeLeft)),
	SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_bWaitingForSpawn ), SendPropBool( SENDINFO_ARRAY( m_bWaitingForSpawn ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_fSpawnTime ), SendPropFloat( SENDINFO_ARRAY( m_fSpawnTime ) ) ),
	SendPropInt( SENDINFO( m_iGametype ) ),

	// map settings data
	SendPropInt( SENDINFO( m_nCommonwealthNation ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nAxisNation ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nCommonwealthUnit ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nAxisUnit ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nAxisInsignia ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nCommonwealthInsignia ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nYear ) ),
	SendPropInt( SENDINFO( m_nSeason ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nTheatre ), 8, SPROP_UNSIGNED ),

	SendPropInt( SENDINFO( m_iRound ), 8, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flRoundStartTime ) ),
	SendPropBool( SENDINFO( m_bOvertime ) ),
	SendPropBool( SENDINFO( m_bSuddenDeath ) ),
	SendPropBool( SENDINFO( m_bFreezeTime ) ),

	//Classlimit vars - Ztormi
	SendPropInt( SENDINFO( m_iCommonwealthMachinegunnerlimit ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iCommonwealthAssaultlimit ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iCommonwealthRiflelimit ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iWehrmachtMachinegunnerlimit ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iWehrmachtAssaultlimit ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iWehrmachtRiflelimit ), 8, SPROP_UNSIGNED ),
	//---end Ztormi

	SendPropInt( SENDINFO( m_iAttackingTeam ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iDefendingTeam ), 8, SPROP_UNSIGNED ),

	SendPropBool( SENDINFO( m_bFreeplay ) ),
	SendPropBool( SENDINFO( m_bOnEndGameScreen ) ),

	SendPropEHandle( SENDINFO(m_GamemodeHandle ) ),
#endif

END_NETWORK_TABLE()

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS( haj_gamerules, CHajGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED(HajGameRulesProxy, DT_HajGameRulesProxy )

/////////////////////////////////////////////////////////////////////////////
#ifdef CLIENT_DLL
	void RecvProxy_HajGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHajGameRules *pRules = HajGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHajGameRulesProxy, DT_HajGameRulesProxy )
		RecvPropDataTable( "haj_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HajGameRules ), RecvProxy_HajGameRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HajGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHajGameRules *pRules = HajGameRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CHajGameRulesProxy, DT_HajGameRulesProxy )
		SendPropDataTable( "haj_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HajGameRules ), SendProxy_HajGameRules )
	END_SEND_TABLE()
#endif

/////////////////////////////////////////////////////////////////////////////
// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	// *HAJ 020* - Jed
	// Do we need a way to have these localised?
	"Commonwealth Forces",
	"Axis Forces",
	"Auto Assign", // needed for autoassign being different to unassigned
};

extern ConVar haj_freeplaytime;

/////////////////////////////////////////////////////////////////////////////
// SHARED
/////////////////////////////////////////////////////////////////////////////
CHajGameRules::CHajGameRules()
{
#ifndef CLIENT_DLL
	
	m_iAttackingTeam = TEAM_INVALID;
	m_iDefendingTeam = TEAM_INVALID;

	m_flTeamCheck = 0.0f;
	m_flPerformTeamBalance = -1.0f;

	m_iRespawnTimes[TEAM_CWEALTH] =	haj_wave_time.GetFloat();
	m_iRespawnTimes[TEAM_AXIS] = haj_wave_time.GetFloat();

	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( sTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	// *HAJ 020* - Jed
	// Hardwire teamplay (we're not a deathmatch game)
	//m_bTeamPlayEnabled = teamplay.GetBool();
	m_bTeamPlayEnabled = true;
	
	m_flIntermissionEndTime = 0.0f;
	m_flGameStartTime = 0;
	m_bSuddenDeath = false;
	m_bFreezeTime = false;

	m_flRoundStartTime = gpGlobals->curtime;

	// bounds check
	if ( mp_timelimit.GetInt() < 0 )
	{
		mp_timelimit.SetValue( 0 );
	}
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	m_pRoundTimerOverride = NULL;
	m_pGamemode = NULL;
	m_roundTimeLeft = -1;

	m_resetRoundEndTime = 0.0f;
	m_bRoundOver = false;

	// map settings defaults
	m_nCommonwealthNation = 0;
	m_nAxisNation = 0;
	m_nCommonwealthUnit = 0;
	m_nAxisUnit = 0;
	m_nCommonwealthInsignia = 0;
	m_nAxisInsignia = 0;
	m_nYear = 0;
	m_nSeason = 0;
	m_nTheatre = 0;

	m_iRound = 1;
	m_bFreeplay = false;
	m_bOvertime = false;
	m_bSwitchTeamsOnRoundReset = false;

	if( haj_freeplaytime.GetInt() > 0 )
	{
		m_bFreeplay = true;
	}

	// global
	g_pGameRules = this;

	//Classlimit vars - Ztormi
	m_iCommonwealthMachinegunnerlimit = -1;
	m_iCommonwealthAssaultlimit = -1;
	m_iCommonwealthRiflelimit = -1;

	m_iWehrmachtMachinegunnerlimit = -1;
	m_iWehrmachtAssaultlimit = -1;
	m_iWehrmachtRiflelimit = -1;
	//end-Ztormi

	m_bOnEndGameScreen = false;

	//Load the classlimits from map script file - Ztormi
	LoadMapScript();

	TheNavMesh->SetPlayerSpawnName( "info_player_cwealth");
#endif

	m_bParsedTeamInfo = false;

	m_PlayerItems.Purge();
	SetupInventoryItems();
}

/////////////////////////////////////////////////////////////////////////////
CHajGameRules::~CHajGameRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

/////////////////////////////////////////////////////////////////////////////
const CViewVectors* CHajGameRules::GetViewVectors()const
{
	return &_hajViewVectors;
}

/////////////////////////////////////////////////////////////////////////////
const CHajViewVectors* CHajGameRules::GetCHajViewVectors()const
{
	return &_hajViewVectors;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::CreateStandardEntities( void )
{
#ifndef CLIENT_DLL
	m_fSpawnTime.Set( TEAM_AXIS, 0.0f);
	m_fSpawnTime.Set( TEAM_CWEALTH, 0.0f);

	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

	// *HAJ 020* - Jed
	g_pLastAxisSpawn = NULL;
	g_pLastCWealthSpawn = NULL;

	// *HaJ 020* - SteveUK

#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "haj_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

void CHajGameRules::AddInventoryItem( const char *entName, itemType_e itemType, int itemQuantity, int iAmmoIndex )
{
	inventoryItem_t newItem;
	Q_snprintf( newItem.itemName, sizeof( newItem.itemName ), "%s", entName );
	newItem.itemType = itemType;
	newItem.itemQuantity = itemQuantity;
	newItem.ammoIndex = iAmmoIndex;

	m_PlayerItems.AddToTail( newItem );
}

void CHajGameRules::SetupInventoryItems( void )
{
	// cwealth weapons
	AddInventoryItem( "weapon_fsknife" );
	AddInventoryItem( "weapon_sten" );
	AddInventoryItem( "weapon_sten5" );
	AddInventoryItem( "weapon_thompson" );
	AddInventoryItem( "weapon_enfield" );
	AddInventoryItem( "weapon_bren" );
	AddInventoryItem( "weapon_bren2" );

	AddInventoryItem( "weapon_millsbomb", ITEM_TYPE_LIMITED_ITEM, 3, GetAmmoDef()->Index( "mills_grenade" ) );
	AddInventoryItem( "weapon_no77smoke", ITEM_TYPE_LIMITED_ITEM, 1, GetAmmoDef()->Index( "smoke_grenade" ) );

	// axis weapons
	AddInventoryItem( "weapon_shovel" ); 
	AddInventoryItem( "weapon_mp40" );
	AddInventoryItem( "weapon_kar98" );
	AddInventoryItem( "weapon_mg34" );

	AddInventoryItem( "weapon_stielgranate", ITEM_TYPE_LIMITED_ITEM, 3, GetAmmoDef()->Index( "stick_grenade" ) );
	AddInventoryItem( "weapon_nb39smoke", ITEM_TYPE_LIMITED_ITEM, 1, GetAmmoDef()->Index( "smoke_grenade" ) );


}

inventoryItem_t* CHajGameRules::GetInventoryItem( const char *entName )
{
	for( int i = 0; i < m_PlayerItems.Count(); i++ )
	{
		inventoryItem_t item = m_PlayerItems.Element( i );

		if( FStrEq( item.itemName, entName ) )
			return &m_PlayerItems.Element(i);
	}

	return NULL;
}

void CHajGameRules::GetPlayerLoadout( CUtlVector<inventoryItem_t *> &pItems, int team, int pclass )
{
	switch( team )
	{
		case TEAM_AXIS: // Axis items

			switch( pclass )
			{
				case CLASS_ASSAULT:
					pItems.AddToTail( GetInventoryItem( "weapon_mp40" ) );
					pItems.AddToTail( GetInventoryItem( "weapon_nb39smoke" ) );
					break;

				case CLASS_RIFLEMAN:
					pItems.AddToTail( GetInventoryItem( "weapon_kar98" ) );
					break;

				case CLASS_SUPPORT:
					pItems.AddToTail( GetInventoryItem( "weapon_mg34" ) );
					break;
			}

			pItems.AddToTail( GetInventoryItem( "weapon_shovel" ) ); // standard melee weapon
			pItems.AddToTail( GetInventoryItem( "weapon_stielgranate" ) );

			return;
			break;

		case TEAM_CWEALTH: // CWealth items

			switch( pclass )
			{
				case CLASS_ASSAULT:
					
					if( GetYear() < YEAR_1941 || GetCommonwealthUnit() == UNIT_CWEALTH_COMMANDO )
						pItems.AddToTail( GetInventoryItem( "weapon_thompson" ) );  // Before 1941 we all get Thompsons, all the time if we're Commando
					else if( GetYear() > YEAR_1943 && GetCommonwealthUnit() == UNIT_CWEALTH_AIRBORNE && GetCommonwealthNation() == NATION_CWEALTH_BRITAIN )
						pItems.AddToTail( GetInventoryItem( "weapon_sten5" ) ); // Use the mkV if it's after 1943 and we're British Airbourne
					else
						pItems.AddToTail( GetInventoryItem( "weapon_sten" ) ); // Use the mkII at any other time

					pItems.AddToTail( GetInventoryItem( "weapon_no77smoke" ) );

					break;

				case CLASS_RIFLEMAN:
					pItems.AddToTail( GetInventoryItem( "weapon_enfield" ) );
					break;

				case CLASS_SUPPORT:

					if( GetYear() <= YEAR_1941 )
						pItems.AddToTail( GetInventoryItem( "weapon_bren" ) ); // mk1 when it's 1941 or before
					else
						pItems.AddToTail( GetInventoryItem( "weapon_bren2" ) );

					break;
				
			}

			pItems.AddToTail( GetInventoryItem( "weapon_fsknife" ) );
			pItems.AddToTail( GetInventoryItem( "weapon_millsbomb" ) );
			
			return;
			break;
	}
}

const char* CHajGameRules::GetPlayerModel( int iTeam, int iClass )
{
	iClass--;

	if ( iTeam == TEAM_CWEALTH )
	{
		switch ( GetCommonwealthNation() )
		{
			case NATION_CWEALTH_CANADA:
				return g_ppszCanadianModels[iClass];
				break;

			case NATION_CWEALTH_POLAND:
				return g_ppszPolishModels[iClass];
				break;

			default:
			case NATION_CWEALTH_BRITAIN:
				return g_ppszBritishModels[iClass];
				break;
		}
	}

	// Axis
	else
	{
		switch ( GetAxisNation() )
		{
			default:
			case NATION_AXIS_GERMANY:
				return g_ppszGermanModels[iClass];
				break;
		}
	}

	return "";
}

extern ConVar haj_support_grenades;
extern ConVar haj_assault_grenades;
extern ConVar haj_rifleman_grenades;
int CHajGameRules::GetClampedStartingAmmoForClass( int iAmmoType, int iDefaultAmmo, int iTeam, int iClass )
{
	// grenade limits
	if( iAmmoType == GetAmmoDef()->Index( "stick_grenade" ) || iAmmoType == GetAmmoDef()->Index( "mills_grenade" ) )
	{
		switch( iClass )
		{
			case CLASS_RIFLEMAN:
				return min( iDefaultAmmo, haj_rifleman_grenades.GetInt() );

			case CLASS_ASSAULT:
				return min( iDefaultAmmo, haj_assault_grenades.GetInt() );

			case CLASS_SUPPORT:
				return min( iDefaultAmmo, haj_support_grenades.GetInt() );
		}
	}

	return iDefaultAmmo;
}

/////////////////////////////////////////////////////////////////////////////
bool CHajGameRules::IsIntermission( void )
{
#ifndef CLIENT_DLL
	return m_flIntermissionEndTime > gpGlobals->curtime;
#endif

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{ 
#ifndef CLIENT_DLL

	int team = pVictim->GetTeamNumber();
	//CHajPlayer *pPlayer = ToHajPlayer( pVictim );

	if( !IsRoundBased() && gpGlobals->curtime > m_fSpawnTime.Get( team ) )
	{
		if( !m_bFreeplay && !m_bSuddenDeath && !m_bFreezeTime )
		{
			float respawnTime = gpGlobals->curtime + haj_wave_time.GetFloat() + 1;

			if( m_iRespawnTimes[ team ] > 0 )
				respawnTime = gpGlobals->curtime + m_iRespawnTimes[ team ] + 1;

			m_fSpawnTime.Set( team, respawnTime ); 
		}
	}

	DeathNotice( pVictim, info );

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	
	CHajPlayer *pScorer = ToHajPlayer( GetDeathScorer( pKiller, pInflictor ) );
	CHajPlayer *pHajVictim = ToHajPlayer( pVictim );
		
	// dvsents2: uncomment when removing all FireTargets
	// variant_t value;
	// g_EventQueue.AddEvent( "game_playerdie", "Use", value, 0, pVictim, pVictim );
	FireTargets( "game_playerdie", pVictim, pVictim, USE_TOGGLE, 0 );

	if( IsFreeplay() )
		return;
	
	pVictim->IncrementDeathCount( 1 );

	// Did the player kill himself?
	if ( pVictim == pScorer )  
	{
		// Players lose a frag for killing themselves
		pVictim->IncrementFragCount( -2 );
		pHajVictim->SendNotification( "#HaJ_Suicide", NOTIFICATION_BASIC, -2 );

	}
	else if ( pScorer )
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		int iPoints = IPointsForKill( pScorer, pVictim );

		pScorer->IncrementFragCount( iPoints );

		if( pScorer->GetTeamNumber() == pVictim->GetTeamNumber() )
		{
			pVictim->IncrementDeathCount( -1 );

			pScorer->SendNotification( "HaJ_YouTKed", NOTIFICATION_MENTIONS_PLAYER, iPoints, pHajVictim, pHajVictim->GetPlayerName() );
			pHajVictim->SendNotification( "#HaJ_Teamkilled", NOTIFICATION_MENTIONS_PLAYER, 0, pScorer, pScorer->GetPlayerName() );
		}
		else
		{
			pVictim->IncrementFragCount( -1 );

			pScorer->SendNotification( "#HaJ_YouKilled", NOTIFICATION_MENTIONS_PLAYER, iPoints, pHajVictim, pHajVictim->GetPlayerName() );
			pHajVictim->SendNotification( "#HaJ_KilledBy", NOTIFICATION_MENTIONS_PLAYER, -1, pScorer, pScorer->GetPlayerName() );
		}

		
		// Allow the scorer to immediately paint a decal
		pScorer->AllowImmediateDecalPainting();
			// dvsents2: uncomment when removing all FireTargets
		//variant_t value;
		//g_EventQueue.AddEvent( "game_playerkill", "Use", value, 0, pScorer, pScorer );
		FireTargets( "game_playerkill", pScorer, pScorer, USE_TOGGLE, 0 );
	}
#endif
}

#ifndef CLIENT_DLL
//=========================================================
//=========================================================
int CHajGameRules::IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	if ( !pKilled )
		return 0;

	if ( !pAttacker )
		return 1;

	if ( pAttacker != pKilled && PlayerRelationship( pAttacker, pKilled ) == GR_TEAMMATE )
		return -2;

	if( pAttacker )
	{
		CHajPlayer *pPlayer = ToHajPlayer( pAttacker );
		
		if( pPlayer )
			return pPlayer->GetNearbyTeammates() + 1; // nearby teammates points
	}

	return 1;
}

bool CHajGameRules::FPlayerCanRespawn( CBasePlayer *pPlayer )
{
	CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

	if( pHajPlayer )
	{
		int team = pPlayer->GetTeamNumber();
		float spawntime = m_fSpawnTime.Get( team );

		if( m_bSuddenDeath || IsRoundBased() )
			return false; // never ever respawn in sudden death
		else if( gpGlobals->curtime >= spawntime && !IsInOvertime() && !m_bRoundOver )
			return true;
	}

	return false;
}
#endif

extern ConVar haj_attackdefend_roundlimit;
extern ConVar haj_switchteams;

/////////////////////////////////////////////////////////////////////////////
extern ConVar haj_autoteambalance;
extern ConVar haj_autoteambalance_sortdelay;
void CHajGameRules::Think( void )
{
#ifndef CLIENT_DLL
	
	CGameRules::Think();

	if( m_bFreeplay && gpGlobals->curtime >= haj_freeplaytime.GetInt() )
	{
		ResetRound();
		m_bFreeplay = false;
	}

	CheckFreezeTimer();
	CheckTeamBalance();

	// check if we need to change the level
	if ( g_fGameOver )
	{
		// check if intermission is over
		// and if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel();
		}

		return;
	}

	// check if time has expired
	if ( m_bRoundOver )
	{
		RoundOverThink();
	}

	m_roundTimeLeft = GetRoundTimeLeft();

#endif
}

#ifndef CLIENT_DLL
extern ConVar haj_round_freezeplayers;
extern ConVar haj_round_intermission;
void CHajGameRules::RoundOverThink()
{
	int roundLimit = 1;

	if(m_pRoundTimerOverride)
		roundLimit = m_pRoundTimerOverride->GetRoundLimit();
	else 
		roundLimit = haj_attackdefend_roundlimit.GetInt();

	// intermission check
	if( ( GetMapRemainingTime() < 0 || ( (roundLimit != 0 && m_iRound >= roundLimit) || HasReachedScoreLimit() ) ) && m_resetRoundEndTime < gpGlobals->curtime )
	{
		GoToIntermission();
		return;
	}

	// check whether to reset round
	if( !g_fGameOver )
	{
		if(m_resetRoundEndTime < gpGlobals->curtime)
		{
			ResetRound();
		}
	}
}

bool CHajGameRules::HasReachedScoreLimit()
{
	CHajGameMode *pGamemode = GetGamemode();

	if( !pGamemode )
		return false;

	int iScoreLimit = pGamemode->GetScoreLimit();

	if( iScoreLimit <= 0 )
		return false;

	if( g_Teams[TEAM_CWEALTH]->GetScore() >= iScoreLimit )
		return true;

	if( g_Teams[TEAM_AXIS]->GetScore() >= iScoreLimit )
		return true;

	return false;
}

void CHajGameRules::CheckFreezeTimer( void )
{
	if( m_bFreezeTime && gpGlobals->curtime >= m_flRoundStartTime + haj_freezetime.GetFloat() )
	{
		m_bFreezeTime = false;

		for ( int i = 0; i < MAX_PLAYERS; i++ )
		{
			CHajPlayer *pPlayer = ToHajPlayer( UTIL_PlayerByIndex( i ) );

			if ( !pPlayer )
				continue;

			pPlayer->RemoveFlag( FL_ATCONTROLS );
			//pPlayer->SetPlayerClass();
		}
	}
}

void CHajGameRules::CheckTeamBalance( void )
{
	// CHECK TEAMS
	if( haj_autoteambalance.GetBool() && !g_fGameOver )
	{
		// CHECK FOR UNBALANCED TEAMS
		if( m_flPerformTeamBalance == -1.0f && gpGlobals->curtime >= m_flTeamCheck )
		{
			CHajTeam *pCWealth = (CHajTeam*)( g_Teams[ TEAM_CWEALTH ] );
			CHajTeam *pAxis = (CHajTeam*)( g_Teams[ TEAM_AXIS ] );

			if( pCWealth && pAxis )
			{
				int iCwealthCount = pCWealth->GetNumPlayers();
				int iAxisCount = pAxis->GetNumPlayers();

				if( !( iCwealthCount == iAxisCount || iCwealthCount - 1 == iAxisCount || iAxisCount - 1 == iCwealthCount ) )
				{
					m_flPerformTeamBalance = gpGlobals->curtime + haj_autoteambalance_sortdelay.GetFloat();

					char delay[20];
					Q_snprintf( delay, sizeof( delay ), "%d", haj_autoteambalance_sortdelay.GetInt() );

					UTIL_ClientPrintAll( HUD_PRINTCENTER, "#HaJ_BalancingTeamsIn", delay );
				}
			}

			m_flTeamCheck = gpGlobals->curtime + 1.5f;
		}

		// PERFORM TEAM BALANCE
		if( m_flPerformTeamBalance != -1.0f && gpGlobals->curtime >= m_flPerformTeamBalance )
		{
			BalanceTeams();
			m_flPerformTeamBalance = -1.0f;
		}
	}
}
#endif

//-----------------------------------------------------------------------------
// *HAJ 020* - Ztormi
// Purpose: Get the classlimits from scriptfile
//-----------------------------------------------------------------------------
void CHajGameRules::LoadMapScript( void )
{
	#ifndef CLIENT_DLL
	char szMapName[MAX_MAP_NAME];
	char szFileName[MAX_PATH];

	KeyValues *pkv = new KeyValues( "Mapinfo" );
	Assert( pkv );

	Q_strncpy( szMapName, STRING(gpGlobals->mapname) ,MAX_MAP_NAME);

	Q_snprintf( szFileName, sizeof( szFileName ), "maps/%s.txt", szMapName);

	// Load the map script file
	if( pkv->LoadFromFile( filesystem, szFileName, "MOD" ) )
	{
		DevMsg("Loading map script file from %s \n",szFileName );
	}
	else
	{
		Warning( "Unable to load map script file for '%s'\n", szFileName  );
		return;
	}

	//Read the keyvalues from file
	pkv->FindKey( "Mapinfo", false );
	if( pkv )
	{
			DevMsg("Loading the classlimits\n");

			m_iCommonwealthRiflelimit = pkv->GetInt( "commonwealth_classlimit_rifle", -1 );
			m_iCommonwealthAssaultlimit = pkv->GetInt( "commonwealth_classlimit_assault", -1 );
			m_iCommonwealthMachinegunnerlimit = pkv->GetInt( "commonwealth_classlimit_mg", -1 );

			m_iWehrmachtRiflelimit = pkv->GetInt( "wehrmacht_classlimit_rifle", -1 );
			m_iWehrmachtAssaultlimit = pkv->GetInt( "wehrmacht_classlimit_assault", -1 );
			m_iWehrmachtMachinegunnerlimit = pkv->GetInt( "wehrmacht_classlimit_mg", -1 );

			DevMsg( "COMMONWEALTH: Rifle %d, Assault %d, MG %d\n", m_iCommonwealthRiflelimit, m_iCommonwealthAssaultlimit, m_iCommonwealthMachinegunnerlimit );
			DevMsg( "AXIS: Rifle %d, Assault %d, MG %d\n", m_iWehrmachtRiflelimit, m_iWehrmachtAssaultlimit, m_iWehrmachtMachinegunnerlimit );
	}
	pkv->deleteThis();

#endif
}

void CHajGameRules::ParseTeamInfo( void )
{
	if( !(m_bParsedTeamInfo && m_pTeamInfo[TEAM_CWEALTH]->CheckConsistency() && m_pTeamInfo[TEAM_AXIS]->CheckConsistency() ) )
	{
		KeyValues *pkv = new KeyValues( "TeamInfo" );

		// Load team info
		if( pkv->LoadFromFile( filesystem, "scripts/teaminfo.txt", "MOD" ) )
		{
			if( !m_pTeamInfo[TEAM_CWEALTH] )
				m_pTeamInfo[TEAM_CWEALTH] = new CTeamInfo( TEAM_CWEALTH );

			if( !m_pTeamInfo[TEAM_AXIS] )
				m_pTeamInfo[TEAM_AXIS] = new CTeamInfo( TEAM_AXIS );

			m_pTeamInfo[TEAM_CWEALTH]->ParseKeyValues( pkv );
			m_pTeamInfo[TEAM_AXIS]->ParseKeyValues( pkv );

			m_bParsedTeamInfo = true;
		}
		else
		{
			Warning( "Failed to find file scripts/teaminfo.txt\n");
		}
	}
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Ztormi
// Purpose: Returns the classlimits
//-----------------------------------------------------------------------------
int CHajGameRules::GetClasslimitForClass( int iClass, int iTeam )
{
	if ( iTeam == TEAM_CWEALTH )
	{
		if ( iClass == CLASS_RIFLEMAN )
			return m_iCommonwealthRiflelimit;
		else if ( iClass == CLASS_ASSAULT )
			return m_iCommonwealthAssaultlimit;
		else if ( iClass == CLASS_SUPPORT )
			return m_iCommonwealthMachinegunnerlimit;
		else
		{
			return -1;
		}
	}
	else if ( iTeam == TEAM_AXIS )
	{
		if ( iClass == CLASS_RIFLEMAN )
			return m_iWehrmachtRiflelimit;
		else if ( iClass == CLASS_ASSAULT )
			return m_iWehrmachtAssaultlimit;
		else if ( iClass == CLASS_SUPPORT )
			return m_iWehrmachtMachinegunnerlimit;
		else
		{
			return -1;
		}
	}

	return -1;

}
//Ztormi---end

/////////////////////////////////////////////////////////////////////////////
extern ConVar haj_intermission_length;
void CHajGameRules::GoToIntermission( void )
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )
		return;

	g_fGameOver = true;
	m_bOnEndGameScreen = true;

	m_flIntermissionEndTime = gpGlobals->curtime + haj_intermission_length.GetFloat();

	if( !m_pGamemode )
		return;

	m_pGamemode->m_eOnGameEndScreen.FireOutput( NULL, NULL );

	CHajTeam* pCWealth = (CHajTeam*)g_Teams[ TEAM_CWEALTH ];
	CHajTeam* pAxis = (CHajTeam*)g_Teams[ TEAM_AXIS ];

	if( !pCWealth || !pAxis )
		return;

	char szMessage[256];
	int iWinningTeam = TEAM_UNASSIGNED;

	if( pCWealth->GetScore() > pAxis->GetScore() ) // CWealth win
	{
		Q_snprintf(szMessage, 256, "%s won the match (%d - %d)", pCWealth->GetName(), pCWealth->GetScore(), pAxis->GetScore());
		m_pGamemode->m_eOnCWealthGameWin.FireOutput( NULL, NULL );

		iWinningTeam = TEAM_CWEALTH;
	}
	else if( pAxis->GetScore() > pCWealth->GetScore() ) // Axis win
	{
		m_pGamemode->m_eOnAxisGameWin.FireOutput( NULL, NULL );
		Q_snprintf(szMessage, 256, "%s won the match (%d - %d)", pAxis->GetName(), pAxis->GetScore(), pCWealth->GetScore());

		iWinningTeam = TEAM_AXIS;

	}
	else // Draw
	{
		Q_snprintf(szMessage, 256, "Game draw! (%d - %d)", pCWealth->GetScore(), pAxis->GetScore());
		m_pGamemode->m_eOnGameDraw.FireOutput( NULL, NULL );
	}

	UTIL_ClientPrintAll(HUD_PRINTTALK, szMessage );

	char nextmap[150];
	GetNextLevelName( nextmap, sizeof( nextmap ) );

	// send event
	IGameEvent * event = gameeventmanager->CreateEvent( "game_end" );
	if ( event )
	{
		event->SetInt( "winning_team", iWinningTeam );
		event->SetString( "next_map", nextmap );
		event->SetInt(	"priority", 7 );

		gameeventmanager->FireEvent( event );
	}

	// freeze all players at intro camera
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->Spawn();

		if( pPlayer->GetViewModel() )
			pPlayer->GetViewModel()->AddEffects( EF_NODRAW );

		pPlayer->RemoveAllItems( true );
		pPlayer->SetActiveWeapon( NULL );

		pPlayer->AddFlag( FL_FROZEN );
		pPlayer->AddEffects( EF_NODRAW );
		pPlayer->SetMoveType( MOVETYPE_NONE );
		pPlayer->AddSolidFlags( FSOLID_NOT_SOLID );

		IPhysicsObject *pObj = pPlayer->VPhysicsGetObject();
		if ( pObj )
			pObj->Sleep();
	}
#endif
	
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::TeamVictoryResetRound(int winningTeamId)
{
#ifndef CLIENT_DLL

	if(m_bRoundOver)
		return;

	if( winningTeamId == -1 )
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#HaJ_GameDraw" );
	}
	else
	{
		CHajTeam* pTeam = (CHajTeam*)g_Teams[winningTeamId];
		const char* szTeamName = pTeam->GetName();

		pTeam->AddScore( 1 );

		if( m_pGamemode != NULL )
			UTIL_ClientPrintAll(HUD_PRINTCENTER, m_pGamemode->GetWinString( winningTeamId ), szTeamName );
		else
			UTIL_ClientPrintAll(HUD_PRINTCENTER, "#HaJ_Win_Generic", szTeamName );
	}

	m_bRoundOver = true;
	m_resetRoundEndTime = gpGlobals->curtime + haj_round_intermission.GetFloat();

	if( haj_switchteams.GetInt() > 0 )
		m_bSwitchTeamsOnRoundReset = (bool)( m_iRound % haj_switchteams.GetInt() == 0 );

	if( m_bSwitchTeamsOnRoundReset )
		UTIL_ClientPrintAll( HUD_PRINTTALK, "#HaJ_RoundTeamSwitchWarn" );

	// fire gamemode outputs for winning
	if( m_pGamemode )
	{
		m_pGamemode->m_eOnRoundEnd.FireOutput( NULL, NULL);

		if( winningTeamId == TEAM_CWEALTH )
			m_pGamemode->m_eOnCWealthWin.FireOutput( NULL, NULL );
		else if( winningTeamId == TEAM_AXIS )
			m_pGamemode->m_eOnAxisWin.FireOutput( NULL, NULL );
	}

	// send event
	IGameEvent *event = gameeventmanager->CreateEvent( "round_win" );
	if( event )
	{
		event->SetInt( "teamid", winningTeamId );
		event->SetFloat( "timeleft", m_pRoundTimerOverride->GetRoundTimeLeft() );
		event->SetBool( "switchingsides", m_bSwitchTeamsOnRoundReset );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}

	if( haj_round_freezeplayers.GetBool() )
	{
		FreezeAllPlayers();
	}

#endif
}

#ifndef CLIENT_DLL
void CHajGameRules::FreezeAllPlayers( void )
{
	// freeze all players at intro camera
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer || !pPlayer->IsAlive() || pPlayer->IsObserver() )
			continue;

		pPlayer->AddFlag( FL_FROZEN );
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	// Msg( "CLIENT DISCONNECTED, REMOVING FROM TEAM.\n" );

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		// Remove the player from his team
		if ( pPlayer->GetTeam() )
		{
			pPlayer->GetTeam()->RemovePlayer( pPlayer );
		}
	}

	BaseClass::ClientDisconnected( pClient );

#endif
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	// Custom kill type?
	if ( info.GetCustomKill() )
	{
		killer_weapon_name = GetCustomKillString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						pInflictor = pScorer->GetActiveWeapon();
						killer_weapon_name = pInflictor->GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( info.GetDamageType() == DMG_DROWN )
			killer_weapon_name = "drowning";
		else if( info.GetDamageType() == DMG_FALL )
			killer_weapon_name = "fall";
		else if( info.GetDamageType() == DMG_SHOCK )
			killer_weapon_name = "shock";
		else if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetString( "inflictor", pInflictor->GetClassname() );
		event->SetInt("damagetype", info.GetDamageType() );
		event->SetInt( "hitgroup", pVictim->LastHitGroup() );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}
#endif

}

#ifndef CLIENT_DLL
const char * CHajGameRules::GetCustomKillString( const CTakeDamageInfo &info )
{
	switch( info.GetCustomKill() )
	{
		case HAJ_KILL_GRENADE_IMPACT:
			return "grenade_impact";

		case HAJ_KILL_MG_OVERHEAT:
			return "overheated_mg";

		case HAJ_KILL_OBJECTIVE:
			return "obj_explosion";

		case HAJ_KILL_SMOKEGREN_BURN:
			return "smokegren_burn";

		case HAJ_KILL_CHOKING_HAZARD:
			return "choking_hazard";

		default:
			return NULL;
	}

	return NULL; // should never get here but w/e
}
#endif

float CHajGameRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	pPlayer->m_Local.m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	float fDamage = pPlayer->m_Local.m_flFallVelocity * ( DAMAGE_FOR_FALL_SPEED * 3.0 );

	pPlayer->ViewPunch( QAngle( -( fDamage ), 0, 0 ) );

	return fDamage;
}

extern ConVar friendlyfire;
ConVar haj_ff_reflect( "haj_ff_reflect", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Reflect team damage?" );
ConVar haj_ff_reflect_ratio( "haj_ff_reflect_ratio", "0.25", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Amount of damage reflected");
extern ConVar haj_freeplay_ff;

#ifndef CLIENT_DLL
bool CHajGameRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if( m_bOnEndGameScreen )
		return false;

	if( ( m_bFreeplay && haj_freeplay_ff.GetBool() ) || pAttacker == pPlayer )
		return true;

	if( pAttacker && PlayerRelationship( pPlayer, pAttacker ) == GR_TEAMMATE && friendlyfire.GetBool() && haj_ff_reflect.GetBool() )
	{
		return false;
	}

	return BaseClass::FPlayerCanTakeDamage( pPlayer, pAttacker );
}
#endif

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;

	// *HAJ 020* - SteveUK
	//	I've removed the cl_playermodel shit because it doesn't actually crash the game at all.
	//  And it stops the glitch where you die and go to team "Unassigned" if you change a setting.

	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}

	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

/////////////////////////////////////////////////////////////////////////////
int CHajGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
	{
		return GR_TEAMMATE;
	}
#endif

	return GR_NOTTEAMMATE;
}

/////////////////////////////////////////////////////////////////////////////
const char *CHajGameRules::GetGameDescription( void )
{ 
#ifndef CLIENT_DLL
	if( m_pGamemode )
	{
		static char szGamemodeName[75];
		Q_snprintf( szGamemodeName, sizeof( szGamemodeName ), "%s - Ham and Jam", m_pGamemode->GetGamemodeName() );

		return szGamemodeName;
	}
#endif

	return "Ham and Jam";
} 

/////////////////////////////////////////////////////////////////////////////
float CHajGameRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = (m_flGameStartTime + mp_timelimit.GetInt() * 60.0f ) - gpGlobals->curtime;

	return timeleft;
}

/////////////////////////////////////////////////////////////////////////////
float CHajGameRules::GetRoundTimeLeft()
{
#if GAME_DLL
	float timeleft = -1.0f;

	if(m_pRoundTimerOverride)
		timeleft = m_pRoundTimerOverride->GetRoundTimeLeft();
	else 
		timeleft = GetMapRemainingTime();

	return timeleft;
#else
	return m_roundTimeLeft;
#endif
}

#ifdef GAME_DLL
void CHajGameRules::ExtendTime( float fTime )
{
	// this will only work on gametypes that support it
	if(m_pRoundTimerOverride)
	{
		m_pRoundTimerOverride->ExtendRound( fTime );

		IGameEvent * event = gameeventmanager->CreateEvent( "roundtime_extend" );
		if ( event )
		{
			event->SetFloat("time_added", fTime );
			event->SetInt("priority", 7 );	// HLTV event priority, not transmitted
			
			gameeventmanager->FireEvent( event );
		}
	}
}
#endif 

ConVar haj_bomb_plantanywhere( "haj_bomb_plantanywhere", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED );
#ifdef CLIENT_DLL
#define CBombZone C_BombZone
#endif
bool CHajGameRules::CanPlayerPlantBomb( CBasePlayer* pPlanter )
{
	if( !pPlanter )
		return false; // no player

	if( haj_bomb_plantanywhere.GetBool() )
		return true;

	if( !GetGamemode() )
		return false;

	CHajPlayer *pPlayer = ToHajPlayer( pPlanter );
	CBombZone *pZone = pPlayer->m_hBombZone.Get();

	if( pPlayer->IsInBombZone() && pZone && pZone->CanBeCapturedByTeam( pPlayer->GetTeamNumber() ) )
	{
		return GetGamemode()->CanPlantBomb( pPlayer, pZone );
	}

	return GetGamemode()->CanPlantBomb( pPlayer );
}

float CHajGameRules::GetPlayerPlantTime( CBasePlayer* pPlanter )
{
	if( GetGamemode() != NULL )
		return GetGamemode()->GetPlantTime();

	return 5.0f;
}


float CHajGameRules::GetPlayerDefuseTime( CBasePlayer* pDefuser )
{
	if( GetGamemode() != NULL )
		return GetGamemode()->GetDefuseTime();

	return 5.0f;
}

float CHajGameRules::GetBombTimer( CBasePlayer* pDefuser )
{
	if( GetGamemode() != NULL )
		return GetGamemode()->GetBombTimer();

	return 35.0f;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );
}

/////////////////////////////////////////////////////////////////////////////
bool CHajGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 

}

/////////////////////////////////////////////////////////////////////////////
bool CHajGameRules::ClientCommand(const char *pcmd, CBaseEntity *pEdict )
{

#ifndef CLIENT_DLL

	CHajPlayer *pHajPlayer = ToHajPlayer( pEdict );

	if ( pHajPlayer && FStrEq( pcmd, "menuselect" ) && pHajPlayer->m_iOpenVoiceMenu > 0 )
	{
		int slot = atoi( engine->Cmd_Argv(1) );

		if( slot != 0 && slot != 10 )
		{
			const char* command = pHajPlayer->GetVoiceString( pHajPlayer->m_iOpenVoiceMenu, slot );
			pHajPlayer->DoVoiceCommand( command );
		}

		pHajPlayer->m_iOpenVoiceMenu = 0;
		return true;
	}


	if( BaseClass::ClientCommand(pcmd, pEdict) )
		return true;

	CHL2MP_Player *pPlayer = (CHL2MP_Player *) pEdict;

	if ( pPlayer->ClientCommand( pcmd ) )
		return true;
#endif

	return false;
}

/////////////////////////////////////////////////////////////////////////////
// SERVER
/////////////////////////////////////////////////////////////////////////////
#ifndef CLIENT_DLL
void CHajGameRules::SendNotification( const char* szLocalise, int iNotificationType, int iFilterTeam /*= TEAM_INVALID*/, int iCE /*= 0*/, CBaseEntity *pEnt /*= NULL*/, const char* szEntName /*= NULL */ )
{
	for( int i = 0; i < gpGlobals->maxClients; i++ )
	{
		CHajPlayer *pPlayer = ToHajPlayer( UTIL_PlayerByIndex( i ) );

		if( pPlayer && ( iFilterTeam == TEAM_INVALID || pPlayer->GetTeamNumber() == iFilterTeam  ))
		{
			pPlayer->SendNotification( szLocalise, iNotificationType, iCE, pEnt, szEntName );
		}
	}
}


void CHajGameRules::ParseTeamAccents( int iTeam )
{
	m_Accents[iTeam].RemoveAll();

	KeyValues *pkv = new KeyValues( "accents" );
	const char* szAccentFile = "scripts/player_accents.txt";

	// Load the map script file
	if( pkv->LoadFromFile( filesystem, szAccentFile, "MOD" ) )
	{
		char temp[25];
		Q_snprintf( temp, sizeof( temp), "%d", iTeam );
		KeyValues *pTeamKeys = pkv->FindKey( temp );

		if( pTeamKeys )
		{
			Q_snprintf( temp, sizeof( temp ), "%d", GetTeamNation( iTeam ) );
			KeyValues *pNationKey = pTeamKeys->FindKey( temp );

			if( pNationKey )
			{
				// got team and nation, now parse the list of accents for this nation
				for( KeyValues *pAccentKey = pNationKey->GetFirstSubKey(); pAccentKey != NULL; pAccentKey = pAccentKey->GetNextKey() )
				{
					int idx = m_Accents[iTeam].AddToTail();
					Q_snprintf( m_Accents[iTeam][idx], sizeof( m_Accents[iTeam][idx] ), pAccentKey->GetString() ); // copy the buffer so no problems with scoping
				}
			}
		}
	}
	else
	{
		Warning( "Failed to read accents file." );
	}

	pkv->deleteThis();


}

int CHajGameRules::GetAccentCount( int iTeam )
{
	return m_Accents[iTeam].Count();
}

void CHajGameRules::BalanceTeams( void )
{
	CHajTeam *pCWealth = (CHajTeam*)( g_Teams[ TEAM_CWEALTH ] );
	CHajTeam *pAxis = (CHajTeam*)( g_Teams[ TEAM_AXIS ] );

	CHajTeam *pWeak = NULL;
	CHajTeam *pStrong = NULL;

	if( pCWealth && pAxis )
	{
		int iCwealthCount = pCWealth->GetNumPlayers();
		int iAxisCount = pAxis->GetNumPlayers();

		if( iCwealthCount == iAxisCount || iCwealthCount - 1 == iAxisCount || iAxisCount - 1 == iCwealthCount )
			return; // no disadvantaged team
	
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "#HaJ_BalancingTeams" );

		if( iCwealthCount > iAxisCount )
		{
			pStrong = pCWealth;
			pWeak = pAxis;
		}
		else
		{
			pStrong = pAxis;
			pWeak = pCWealth;
		}

		int iPlayersSwitched = 0;
		int iPlayersToSwitch = ( pStrong->GetNumPlayers() - pWeak->GetNumPlayers() ) / 2;
		int iPlayerIndexLoop = 0;
		//int iPlayersStrongTeam = pStrong->GetNumPlayers();
		//int iPlayersWeakTeam = pWeak->GetNumPlayers();
		int iPlayerIndex = pStrong->GetNumPlayers() - iPlayersToSwitch;

		while( iPlayersSwitched < iPlayersToSwitch  ) // recently joined players selected
		{
			CHajPlayer *pPlayer = NULL;
			
			if( iPlayerIndexLoop < pStrong->GetNumPlayers() )
				pPlayer = ToHajPlayer( pStrong->GetPlayer( iPlayerIndexLoop ) );
			else
			{
				pPlayer = ToHajPlayer( pStrong->GetPlayer( iPlayerIndex ) );
				++iPlayerIndex;
			}

			if( pPlayer && ( iPlayerIndexLoop >= pStrong->GetNumPlayers() || ( iPlayerIndexLoop < pStrong->GetNumPlayers() && !pPlayer->IsAlive() ) ) )
			{
				if( pPlayer->IsAlive() )
				{
					pPlayer->CommitSuicide();

					if( !m_bFreeplay )
					{
						pPlayer->IncrementFragCount( 1 );
						pPlayer->IncrementDeathCount( -1 );
					}
				}

				pPlayer->ChangeTeam( pWeak->GetTeamNumber() );
				pPlayer->SetActiveClass( CLASS_UNASSIGNED );
				pPlayer->SetDesiredClass( CLASS_UNASSIGNED );
				pPlayer->ShowClassMenu();

				ClientPrint( pPlayer, HUD_PRINTTALK, "#HaJ_MovedToOtherTeam_Balance" );
				ClientPrint( pPlayer, HUD_PRINTCENTER, "#HaJ_MovedToOtherTeam_Balance" );

				++iPlayersSwitched;
			}

			++iPlayerIndexLoop;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
CBaseEntity* CHajGameRules::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
	CBaseEntity* pSpawnSpot = BaseClass::GetPlayerSpawnSpot(pPlayer);

	if(pSpawnSpot)
	{
		CHajSpawnPoint* pHajSpawnPoint = dynamic_cast<CHajSpawnPoint*>(pSpawnSpot);
		if(pHajSpawnPoint)
		{
			pHajSpawnPoint->OnPlayerSpawn(pPlayer);
			DevMsg( "Spawned player %s (team %d) at %s (%s), Enabled: %d\n", pPlayer->GetPlayerName(), pPlayer->GetTeamNumber(), pHajSpawnPoint->GetClassname(), pHajSpawnPoint->GetEntityName(), pHajSpawnPoint->IsEnabled() );
		}
	}

	return pSpawnSpot;
}

void CHajGameRules::StartOvertime( float time )
{
	Msg( "Started overtime period: %f seconds\n", time );

	UTIL_ClientPrintAll( HUD_PRINTCENTER, "#HaJ_OvertimeNotify" );
	m_bOvertime = true;
}

void CHajGameRules::StartSuddenDeath( void )
{
	Msg( "Started sudden death\n" );

	UTIL_ClientPrintAll( HUD_PRINTCENTER, "#HaJ_SuddenDeathNotify" );
	m_bOvertime = true;
	m_bSuddenDeath = true;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameRules::ResetRound()
{
	if( haj_autoteambalance.GetBool() )
		BalanceTeams();

	m_bOvertime = false;
	m_bSuddenDeath = false;
	m_flRoundStartTime = gpGlobals->curtime;

	if( haj_freezetime.GetFloat() > 0 )
		m_bFreezeTime = true;

	m_fSpawnTime.Set( TEAM_AXIS, 0.0f);
	m_fSpawnTime.Set( TEAM_CWEALTH, 0.0f);

	// switch team scores
	if( m_bSwitchTeamsOnRoundReset )
	{
		CTeam* pTeamAxis = g_Teams[TEAM_AXIS];
		CTeam* pTeamCWealth = g_Teams[TEAM_CWEALTH];

		if( pTeamAxis && pTeamCWealth )
		{
			int axisScore = pTeamAxis->GetScore();
			int cwealthScore = pTeamCWealth->GetScore();

			pTeamAxis->SetScore( cwealthScore );
			pTeamCWealth->SetScore( axisScore );
		}

		UTIL_ClientPrintAll( HUD_PRINTTALK, "#HaJ_RoundTeamSwitch" );
	}


	// setup a map entity filter to indicate which
	// entities not to destroy
	CMapEntityFilter filter;

/*
"player",
"viewmodel",
"worldspawn",
"soundent",
"ai_network",
"ai_hint",
"env_soundscape",
"env_soundscape_proxy",
"env_soundscape_triggerable",
"env_sprite",
"env_sun",
"env_wind",
"env_fog_controller",
"func_wall",
"func_illusionary",
"info_node",
"info_target",
"info_node_hint",
"point_commentary_node",
"point_viewcontrol",
"func_precipitation",
"func_team_wall",
"shadow_control",
"sky_camera",
"scene_manager",
"trigger_soundscape",
"commentary_auto",
"point_commentary_node",
"point_commentary_viewpoint",
*/

	// destroy the entities we want to reset
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while(pEnt != NULL)
	{
		const char* szClassName = pEnt->GetClassname();
		if(filter.ShouldCreateEntity(szClassName))
			UTIL_Remove(pEnt);

		pEnt = gEntList.NextEnt(pEnt);
	}

	// notify teams of round reset
	int nTeams = g_Teams.Count();
	for(int i = 0; i < nTeams; ++i)
	{
		CHajTeam* pTeam = (CHajTeam*)g_Teams[i];
		pTeam->ResetRound();
	}

	// load the entities as if it was a new map load
	const char* szMapEntities = _serverGameDLL.GetCurrentMapEntities();
	MapEntity_ParseAllEntities(szMapEntities, &filter, true);

	m_bRoundOver = false;

	// go through players
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

		if ( !pHajPlayer )
			continue;

		pHajPlayer->RemoveFlag( FL_FROZEN );

		if( m_bSwitchTeamsOnRoundReset ) // if switch teams is on, change team and prompt for class selection
		{
			if( pHajPlayer->GetTeamNumber() == TEAM_CWEALTH )
			{
				pHajPlayer->ChangeTeam( TEAM_AXIS );
			}
			else if( pHajPlayer->GetTeamNumber() == TEAM_AXIS )
			{
				pHajPlayer->ChangeTeam( TEAM_CWEALTH );
			}

			pHajPlayer->m_lifeState = LIFE_DEAD;
			pHajPlayer->StartObserverMode( OBS_MODE_FIXED );

			pHajPlayer->SetActiveClass( CLASS_UNASSIGNED );
			pHajPlayer->SetDesiredClass( CLASS_UNASSIGNED );

			pHajPlayer->ShowClassMenu();

		}
		else // auto switch isn't on
		{
			if( !(pHajPlayer->GetDesiredClass() == CLASS_UNASSIGNED && pHajPlayer->GetCurrentClass() == CLASS_UNASSIGNED ) )
				pHajPlayer->Spawn(); // spawn if not unassigned class
		}
	}

	if( !m_bFreeplay )
		m_iRound++;

	if( m_pGamemode != NULL )
		m_pGamemode->ResetRound();

	IGameEvent * event = gameeventmanager->CreateEvent( "round_restart" );
	if ( event )
	{
		event->SetInt( "round_number", m_iRound );
		event->SetBool( "team_switch", m_bSwitchTeamsOnRoundReset );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted

		gameeventmanager->FireEvent( event );
	}

}

/////////////////////////////////////////////////////////////////////////////
bool CHajGameRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
{		
	return false;
/*
	if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
	{
		// Player has an active item, so let's check cl_autowepswitch.
		const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
		if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
		{
			return false;
		}
	}

	return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
*/
}

//=========================================================
//=========================================================
bool CHajGameRules::PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker )
{
	Msg( "Invalid pointer in PlayerCanHearChat\n" );
	return ( PlayerRelationship( pListener, pSpeaker ) == GR_TEAMMATE );
}

const char *CHajGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	CHajPlayer* pHajPlayer = ToHajPlayer( pPlayer );

	if( !pHajPlayer )
		return "(RCON)";

	if( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return "(SPECTATOR)";
	else if( !pPlayer->IsAlive() && !bTeamOnly )
		return "(DEAD)";
	else if( !pPlayer->IsAlive() && bTeamOnly )
		return "(DEAD TEAM)";
	else if( pHajPlayer && bTeamOnly && pHajPlayer->IsLocationSet() )
	{
		return pHajPlayer->GetPlayerLocation( true );
	}
	else if ( bTeamOnly )
		return "(TEAM)";
	else
		return "";
}


/////////////////////////////////////////////////////////////////////////////
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
bool CHajGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	int iAmmoIndex = pItem->GetPrimaryAmmoType();
	CBaseCombatWeapon *pAmmoWeapon = pPlayer->Weapon_GetWpnForAmmo( iAmmoIndex );
	CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

	if( pAmmoWeapon && !pItem->IsMeleeWeapon() )
	{
		Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex( pItem->GetPrimaryAmmoType() );

		if( FBitSet( pAmmo->nFlags, AMMO_USE_MAGAZINES ) )
		{
			// get pointer to magazine container
			CHajWeaponMagazines *pMags = pHajPlayer->GetMagazines( iAmmoIndex );

			// don't let us pick up ammo if we are above our magazine limit
			if( !pMags || pMags->MagazineCount() >= pAmmo->pMaxCarry )
				return false;

			if( pAmmoWeapon->m_iClip1 > 0 )
			{
				pMags->AddMag( pAmmoWeapon->m_iClip1 );
				pPlayer->EmitSound( "BaseCombatCharacter.AmmoPickup" );

				UTIL_Remove( pItem );
				return false;
			}
		}
		// not using magazine system, just give current clip or 1 of whatever it is
		else
		{
			pPlayer->GiveAmmo( ( pItem->UsesClipsForAmmo1() ? pItem->m_iClip1 : 1 ), pItem->GetPrimaryAmmoType() );
			
			UTIL_Remove( pItem );
			return false;
		}
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}

//-----------------------------------------------------------------------------
// Purpose: Returns the weapon in the player's inventory that would be better than
//			the given weapon.
//-----------------------------------------------------------------------------
CBaseCombatWeapon *CHajGameRules::GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon )
{
	CBaseCombatWeapon *pCheck;
	CBaseCombatWeapon *pBest;// this will be used in the event that we don't find a weapon in the same category.

	int iCurrentWeight = -1;
	int iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	// If I have a weapon, make sure I'm allowed to holster it
	if ( pCurrentWeapon )
	{
		if ( !pCurrentWeapon->AllowsAutoSwitchFrom() || !pCurrentWeapon->CanHolster() )
		{
			// Either this weapon doesn't allow autoswitching away from it or I
			// can't put this weapon away right now, so I can't switch.
			return NULL;
		}

		iCurrentWeight = pCurrentWeapon->GetWeight();
	}

	for ( int i = 0 ; i < pPlayer->WeaponCount(); ++i )
	{
		pCheck = pPlayer->GetWeapon( i );
		if ( !pCheck )
			continue;

		// If we have an active weapon and this weapon doesn't allow autoswitching away
		// from another weapon, skip it.
		if ( pCurrentWeapon && !pCheck->AllowsAutoSwitchTo() )
			continue;

		if ( pCheck->GetWeight() > -1 && pCheck->GetWeight() == iCurrentWeight && pCheck != pCurrentWeapon )
		{
			// this weapon is from the same category. 
			if ( pCheck->HasAnyAmmo() )
			{
				if ( pPlayer->Weapon_CanSwitchTo( pCheck ) )
				{
					return pCheck;
				}
			}
		}
		else if ( pCheck->GetWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
		{
			//Msg( "Considering %s\n", STRING( pCheck->GetClassname() );
			// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
			// that the player was using. This will end up leaving the player with his heaviest-weighted 
			// weapon. 
			if ( pCheck->HasAnyAmmo() )
			{
				// if this weapon is useable, flag it as the best
				iBestWeight = pCheck->GetWeight();
				pBest = pCheck;
			}
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 
	
	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	return pBest;
}

#endif


bool CHajGameRules::IsRoundBased()
{
	if( GetGamemode() != NULL && GetGamemode()->IsRoundBased() )
		return true;

	return false;
}