
// haj_player.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "haj_cvars.h"
#include "haj_gamerules.h"
#include "haj_player.h"
#include "haj_team.h"
#include "haj_player_shared.h"
#include "haj_weapon_base.h"
#include "haj_weapon_grenadebase.h"
#include "haj_location.h"
#include "haj_pickup.h"
#include "te_effect_dispatch.h"
#include "ilagcompensationmanager.h"
#include "con_nprint.h"
#include "viewport_panel_names.h"

#include "weapon_satchelcharge.h"
#include "haj_mapsettings_enums.h"
#include "haj_objectivemanager.h"
#include "haj_capturepoint.h"

#include "haj_playerhelmet_shared.h"
#include "haj_playerhelmet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar friendlyfire;
extern ConVar haj_ff_reflect;
extern ConVar haj_ff_reflect_ratio;

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
	{
	}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
SendPropEHandle( SENDINFO( m_hPlayer ) ),
SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	//Tony; pull the player who is doing it out of the recipientlist, this is predicted!!
	filter.RemoveRecipient( pPlayer );

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(player, CHajPlayer);

/////////////////////////////////////////////////////////////////////////////
// network data
IMPLEMENT_SERVERCLASS_ST(CHajPlayer, DT_HajPlayer)

	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),

	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	//SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	SendPropEHandle( SENDINFO( m_hPickup ) ),
	SendPropInt( SENDINFO( m_nNearbyTeamMates), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bWeaponLowered ) ),
	SendPropEHandle( SENDINFO( m_hHelmetEnt ) ),
	SendPropInt( SENDINFO( m_nHelmetId ), 6, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bInDeployZone ) ),
	SendPropInt( SENDINFO( m_iDeployZStance ), 8 ),
	SendPropInt( SENDINFO( m_iObjectiveScore ), 12 ),
	SendPropInt( SENDINFO( m_iObjectivesCapped ), 12 ),
	SendPropInt( SENDINFO( m_iObjectivesDefended ), 12 ),
	SendPropBool( SENDINFO( m_bInBombZone ) ),
	SendPropEHandle( SENDINFO( m_hBombZone ) ),
	SendPropFloat( SENDINFO( m_flLastShotTime ) ),
	SendPropVector( SENDINFO( m_vecRenderOrigin ), -1, SPROP_COORD ),
END_SEND_TABLE()

/////////////////////////////////////////////////////////////////////////////
// data table
BEGIN_DATADESC(CHajPlayer)
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
CHajPlayer::CHajPlayer()
{
	m_pHajAnimState = CreateHajPlayerAnimState( this );
	UseClientSideAnimation();

	m_fNextTeamMateCheck = gpGlobals->curtime + 1.0f;
	m_nNearbyTeamMates = 0;
	m_bWeaponLowered = false;
	m_nHelmetId = 0;
	m_fNextLocationUpdate = 0.0f;
	m_bHasHelmet = false;
	m_flLastShotTime = 0.0f;	// *HAJ 020* - Jed Init

	m_iOpenVoiceMenu = 0;
	m_flNextVoiceCommand = 0;

	SetDefLessFunc( m_PlayerMagazines );

	m_hHelmetEnt = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CHajPlayer::~CHajPlayer()
{
	m_pHajAnimState->Release();
}

/////////////////////////////////////////////////////////////////////////////
void CHajPlayer::Precache( void )
{
	PrecacheScriptSound( "Voice.BritGrenade" );
	PrecacheScriptSound( "Voice.GerGrenade" );
	PrecacheScriptSound( "Voice.BritGo" );
	PrecacheScriptSound( "Voice.GerGo" );
	PrecacheScriptSound( "Voice.BritObj" );
	PrecacheScriptSound( "Voice.GerObj" );
	PrecacheScriptSound( "Voice.BritSnip" );
	PrecacheScriptSound( "Voice.GerSnip" );
	PrecacheScriptSound( "Voice.BritHold" );
	PrecacheScriptSound( "Voice.GerHold" );

	PrecacheScriptSound( "Player.Headshot" );

	// precache helmet models
	PrecacheHelmets();

	BaseClass::Precache();
}

/////////////////////////////////////////////////////////////////////////////
CHajPickup* CHajPlayer::GetPickup() const
{
	return dynamic_cast<CHajPickup*>(m_hPickup.Get());
}

//-----------------------------------------------------------------------------
// *HAJ 020* - Jed
// Purpose: Stores the time you last shot a round
//-----------------------------------------------------------------------------
void CHajPlayer::SetLastShotTime( float fTime )
{
	m_flLastShotTime = fTime;
}


/////////////////////////////////////////////////////////////////////////////
bool CHajPlayer::OnPickupTouch(CHajPickup* pPickup)
{
	if(!pPickup || m_hPickup.Get() || ( pPickup->GetCaptureTeam() != -1 && GetTeamNumber() != pPickup->GetCaptureTeam() ) )
		return false;

	IncrementFragCount( 2 );

	m_hPickup.Set((CBaseEntity*)pPickup);
	return true;
}

bool CHajPlayer::OnPickupRecover(CHajPickup* pPickup)
{
	if( GetTeamNumber() != pPickup->GetCaptureTeam() && CHajTeam::IsCombatTeam( GetTeamNumber() ) )
	{
		IncrementFragCount( 1 );
		return true;
	}

	return false;
}

void CHajPlayer::SendHint( const char* langkey )
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	UserMessageBegin( user, "HajPlayerHint" );
		WRITE_STRING( langkey );
	MessageEnd();
}

void CHajPlayer::PlayerDeathThink( )
{
	if( GetTeamNumber() != TEAM_SPECTATOR && IsObserver() && CHajTeam::IsCombatTeam( GetTeamNumber() ) )
	{
		CBaseEntity *pTarget = GetObserverTarget();

		if( GetObserverMode() == OBS_MODE_ROAMING )
		{
			int mode = OBS_MODE_FIXED;
	
			if( pTarget && pTarget->IsPlayer() )
				mode = OBS_MODE_IN_EYE;

			engine->ClientCommand( edict(), "cl_spec_mode %d", mode );

			StartObserverMode( mode );
		}
		else if( GetObserverMode() == OBS_MODE_FIXED )
		{
			CTeam *team = GetTeam();

			if( team->GetNumPlayers() > 1 )
			{
				engine->ClientCommand( edict(), "spec_next" );
				engine->ClientCommand( edict(), "spec_mode %d", OBS_MODE_IN_EYE );
			}
		}
	}

	if( ( GetTeamNumber() == TEAM_CWEALTH || GetTeamNumber() == TEAM_AXIS ) && ( GetActiveClass() == CLASS_UNASSIGNED && GetDesiredClass() == CLASS_UNASSIGNED ) )
	{
		return; // this prevents respawning if the class is still unassigned
	}

	return BaseClass::PlayerDeathThink();
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we can switch to the given weapon.
// Input  : pWeapon - 
//-----------------------------------------------------------------------------
bool CHajPlayer::Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon )
{
	CBasePlayer *pPlayer = (CBasePlayer *)this;
#if !defined( CLIENT_DLL )
	IServerVehicle *pVehicle = pPlayer->GetVehicle();
#else
	IClientVehicle *pVehicle = pPlayer->GetVehicle();
#endif
	if (pVehicle && !pPlayer->UsingStandardWeaponsInVehicle())
		return false;

	if ( !pWeapon->CanDeploy() )
		return false;

	if ( GetActiveWeapon() && !GetActiveWeapon()->CanHolster() )
			return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Lowers the weapon posture (for hovering over friendlies)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayer::Weapon_Lower( void )
{
	CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase *>( GetActiveWeapon() );

	if ( pWeapon == NULL || pWeapon->IsLowered() == true )
		return false;

	return pWeapon->Lower();
}

//-----------------------------------------------------------------------------
// Purpose: Returns the weapon posture to normal
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayer::Weapon_Ready( void )
{
	CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase *>( GetActiveWeapon() );

	if ( pWeapon == NULL || pWeapon->IsLowered() == false )
		return false;

	return pWeapon->Ready();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHajPlayer::PostThink( void )
{
	BaseClass::PostThink();

	CHAJWeaponBase *pWeapon = dynamic_cast< CHAJWeaponBase* >( GetActiveWeapon() );

	if( pWeapon )
	{
		if( IsSprinting() && pWeapon->IsLowered() == false )
		{
			Weapon_Lower();
			SetMaxSpeed( HAJ_SPRINT_SPEED );
		}
		else if( !IsSprinting() && pWeapon->IsLowered() == true )
			Weapon_Ready();
	}

	if ( m_fNextTeamMateCheck < gpGlobals->curtime )
		UpdateNearbyTeamMateCount();

	if( GetNearbyTeammates() > 0 ) // nearby teammate bonuses here
	{
		if( SuitPower_GetCurrentPercentage() < 100 )
			SuitPower_Charge( ( 6.25 * GetNearbyTeammates() ) * gpGlobals->frametime );
	}


	// hacky as fuck, gamemovement doesn't want to play ball with this code so i'll put it here.
	if( m_Local.m_bDoProne || m_Local.m_bDoUnProne || m_Local.m_bProned || m_Local.m_bProning )
	{
		if( MaxSpeed() != HAJ_NORM_SPEED )
			SetMaxSpeed( HAJ_NORM_SPEED );

		if( IsSprinting() )
			StopSprinting();
	}

	// in_grenade
	if( m_nButtons & (IN_GRENADE1|IN_GRENADE2) )
	{
		int iSlot = (m_nButtons & IN_GRENADE1) ? 2 : 3;
		CWeaponGrenade *pGrenade = dynamic_cast<CWeaponGrenade*>(Weapon_GetSlot( iSlot ));

		if( pGrenade && pGrenade != GetActiveWeapon() )
		{
			if( pGrenade->HasAmmo() )
			{
				pGrenade->m_hRestoreLastWeapon.Set( Weapon_GetLast() );
				Weapon_Switch( pGrenade );
			} // could do with an else for this
		}
	}

	m_pHajAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );

}

void CHajPlayer::UpdateLocation( const char* strLocation )
{
	Q_snprintf( m_strLocation, sizeof( m_strLocation ), "%s", strLocation );
}

void CHajPlayer::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}


//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
extern ConVar haj_round_freezeplayers;

void CHajPlayer::Spawn(void)
{
	// make sure old mags are got rid of (round restart)
	for( int i = 0; i < m_PlayerMagazines.Count(); i++ )
	{
		CHajWeaponMagazines *pMagazine = m_PlayerMagazines.Element(i);
		pMagazine->RemoveAll();
	}

	CheckDesired(); // *HaJ 020* SteveUK - this checks our desired team or class settings.
	
	UTIL_Concuss( this, 0.0f, CONCUSS_STOP );

	SetRenderOrigin( vec3_origin ); // just in case we get killed while vaulting

	BaseClass::Spawn();

	DoAnimationEvent( PLAYERANIMEVENT_SPAWN );

	m_iOpenVoiceMenu = 0;
	m_nButtons = 0;

	// reset movement vars
	m_Local.m_bVaulting = false;
	m_Local.m_bVaultingDisabled = false;
	m_Local.m_hVaultHint.Set( NULL );
	m_Local.m_flVaultGroundTime = -1;
	m_Local.m_iVaultHeight = -1;
	m_Local.m_iVaultStage = 0;
	m_Local.m_bProned = false;
	m_Local.m_bProning = false;
	m_Local.m_bDoProne = false;
	m_Local.m_bDoUnProne = false;
	m_Local.m_flNextStanceChange = gpGlobals->curtime;
	m_Local.m_flNextJumpTime = gpGlobals->curtime;

	EnableSprint( true ); // reset disabled sprint

	if( GetTeamNumber() != TEAM_SPECTATOR && GetCurrentClass() != CLASS_UNASSIGNED && GetCurrentClass() != CLASS_SUPPORT )
	{
		SetAmmoCrates( 2 );
	}
	else
	{
		SetAmmoCrates( 0 );
	}

	random->SetSeed( gpGlobals->curtime * GetUserID() );


	if( GetViewModel() )
	{
		GetViewModel()->SetBodygroup( 1, GetTeamNumber() - 1 ); // change hands
	}

	m_bInDeployZone = false; // Ztormi
	m_bWeaponLowered = false;

	CHajGameRules *pGameRules = HajGameRules();

	if( pGameRules )
	{
		if( pGameRules->IsRoundOver() && haj_round_freezeplayers.GetBool() )
			AddFlag( FL_FROZEN );
		else
			RemoveFlag( FL_FROZEN );

		if( pGameRules->IsFreeplay() && GetTeamNumber() != TEAM_SPECTATOR )
			SendHint( "HaJ_Tip_Freeplay" );

		if( pGameRules->IsFreezeTime() )
			AddFlag( FL_ATCONTROLS );

		if( pGameRules->GetAccentCount( GetTeamNumber() ) > 0 )
		{
			m_iAccent = random->RandomInt( 0, pGameRules->GetAccentCount( GetTeamNumber() ) - 1 );
			Msg( "Spawning with accent %d\n", m_iAccent );
		}
		else
			m_iAccent = -1;
	}

	//SetPlayerClass();	// *HAJ 020* - Jed Set class.

	// Set player class
	if( m_iClass > CLASS_UNASSIGNED && !IsObserver() && !g_fGameOver )
	{
		RemoveAllItems( false ); // true includes the HEV suit.

		CUtlVector<inventoryItem_t*> items;
		HajGameRules()->GetPlayerLoadout( items, GetTeamNumber(), m_iClass );

		for( int i = 0; i < items.Count(); i++ )
		{
			inventoryItem_t* pItem = items.Element( i );

			if( pItem )
			{
				int iAmmo = -1;
				if( pItem->itemQuantity > -1 || pItem->itemType == ITEM_TYPE_LIMITED_ITEM )
				{
					iAmmo = HajGameRules()->GetClampedStartingAmmoForClass( pItem->ammoIndex, pItem->itemQuantity, GetTeamNumber(), m_iClass );
				}
				
				if( iAmmo != 0 )
				{
					CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)GiveNamedItem( pItem->itemName );

					if( iAmmo > 0 )
						SetAmmoCount( iAmmo, pItem->ammoIndex );
					else if( pWeapon->UsesClipsForAmmo1() )
					{
						Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex( pWeapon->GetPrimaryAmmoType() );
						GiveAmmo( pAmmo->pMaxCarry, pWeapon->GetPrimaryAmmoType(), true );
					}
				}
			}
		}

		// select weapon in primary weapon slot
		CBaseCombatWeapon *pWeapon = Weapon_GetSlot( 1 );
		if( pWeapon ) Weapon_Switch( pWeapon );

		Weapon_SetLast( Weapon_GetSlot( 0 ) );
	}

	// set current class to chose class
	m_iCurrentClass = m_iClass;

	// set our model
	if( !IsObserver() && m_iClass > CLASS_UNASSIGNED && pGameRules )
		SetModel( pGameRules->GetPlayerModel( GetTeamNumber(), m_iClass ) );

	// put a helmet on your head
	CreateHelmet();

	// call gamemode player spawn func
	if( HajGameRules()->GetGamemode() != NULL )
		HajGameRules()->GetGamemode()->PlayerSpawn( this );

}

void CHajPlayer::InitialSpawn( void )
{
	BaseClass::InitialSpawn();

	StartObserverMode( OBS_MODE_FIXED );
}

bool CHajPlayer::IsValidObserverTarget(CBaseEntity * target)
{
	CHajPlayer *pTarget = ToHajPlayer( target );

	if( pTarget && CHajTeam::IsCombatTeam( GetTeamNumber() ) )
	{
		if( GetTeamNumber() != pTarget->GetTeamNumber() )
			return false;

		if( !pTarget->IsAlive() && pTarget->GetDeathTime() + DEATH_ANIMATION_TIME >= gpGlobals->curtime )
			return false; // invalid shortly after dying
	}

	return BaseClass::IsValidObserverTarget( target );
}

bool CHajPlayer::StartObserverMode(int mode)
{
	if( GetTeamNumber() != TEAM_SPECTATOR && CHajTeam::IsCombatTeam( GetTeamNumber() ) && mode == OBS_MODE_ROAMING )
	{
		mode = OBS_MODE_IN_EYE;
		engine->ClientCommand( edict(), "cl_spec_mode %d", mode );
	}

	return BaseClass::StartObserverMode( mode );
}

void CHajPlayer::SuitPower_Update( void )
{
	BaseClass::SuitPower_Update();
}


extern int	gEvilImpulse101;
extern bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer );
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHajPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
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

	// Don't let the player touch the item unless unobstructed
	if ( !UTIL_ItemCanBeTouchedByPlayer( pWeapon, this ) && !gEvilImpulse101 )
		return false;

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( pWeapon->FVisible( this, MASK_SOLID ) == false && !(GetFlags() & FL_NOTARGET) )
		return false;

	if ( Weapon_SlotOccupied( pWeapon ) )
		return false;

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
		return false;

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

/////////////////////////////////////////////////////////////////////////////

void CHajPlayer::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	CHAJWeaponBase *pWpn = dynamic_cast<CHAJWeaponBase *>( pWeapon );
	
	if ( pWpn )
	{
		pWpn->UnlockBipod();
	}
		
	SetFOV( GetFOVOwner(), 0, 0.1f );

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}

/**
	Implement magazine system when ammo is given to player
*/
int CHajPlayer::GiveAmmo( int nCount, int iAmmoIndex, bool bSuppressSound )
{
	Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex(iAmmoIndex);

	if( !pAmmo )
		return 0;

	// Deal with magazine based ammo
	if( FBitSet( pAmmo->nFlags, AMMO_USE_MAGAZINES ) )
	{
		// get pointer to magazine container
		CHajWeaponMagazines *pMags = GetMagazines( iAmmoIndex );

		// don't let us pick up ammo if we are above our magazine limit
		if( !pMags || pMags->MagazineCount() >= pAmmo->pMaxCarry )
			return 0;

		int iReturn = pMags->AddMags( nCount );

		// hacks to replicate normal source behaviour
		SetAmmoCount( pMags->MagazineCount()-1, iAmmoIndex );
		if( iReturn > 0 ) EmitSound( "BaseCombatCharacter.AmmoPickup" );

		return iReturn;
	}

	return BaseClass::GiveAmmo( nCount, iAmmoIndex, bSuppressSound );
}

CHajWeaponMagazines* CHajPlayer::GetMagazines( int iAmmoType )
{
	// find index of magazine
	int idx = m_PlayerMagazines.Find( iAmmoType );
	if( !m_PlayerMagazines.IsValidIndex( idx ) ) idx = m_PlayerMagazines.Insert( iAmmoType, new CHajWeaponMagazines( this, iAmmoType ) );

	// get pointer to magazine container
	return m_PlayerMagazines.Element( idx );
}

void CHajPlayer::PlayerUse( void )
{
	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	CWeaponSatchelCharge *pSatchel = dynamic_cast<CWeaponSatchelCharge*>(Weapon_GetSlot( 5 ));
	
	if( pSatchel && pSatchel->HasAmmo() && pSatchel->CanPlant() )
	{
		// switch with +use
		pSatchel->SetLastWeaponWithUse( Weapon_GetLast() );

		if( Weapon_Switch( pSatchel ) )
			pSatchel->SetPlantingWithUse( true );

		return;
	}

	BaseClass::PlayerUse();
}

void CHajPlayer::DefendedObjective( CHajObjective* pObjective, CBasePlayer *pVictim, bool bDefuse /* = false */ )
{
	IncrementFragCount( 3 + m_nNearbyTeamMates );
	IncreaseObjectiveScore( 1 );

	m_iObjectivesDefended++;
	m_flNextDefensiveAction = gpGlobals->curtime + 5.0f;

	IGameEvent * event = gameeventmanager->CreateEvent( "zone_defended" );
	
	if ( event )
	{
		event->SetString( "zone", STRING( pObjective->GetNameOfZone() ) );
		event->SetInt( "entityid", pObjective->entindex() );
		event->SetInt( "teamid", pObjective->GetOwnerTeamId() );
		event->SetInt( "defender", GetUserID() );
		event->SetBool( "defusedtnt", bDefuse );

		gameeventmanager->FireEvent( event );
	}
}

void CHajPlayer::KillOnPoint( CHajObjective* pObj, CBasePlayer *pVictim )
{
	if( GetTeamNumber() != pVictim->GetTeamNumber() )
	{
		IncrementFragCount( 1 ); // give extra frag for killing player whilst trying to cap or running into defend!
		SendNotification( "#HaJ_PointKill", NOTIFICATION_BASIC, 1 );
	}
}

void CHajPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	m_bWeaponLowered = false;

	// GRENADE DROPPING
	CWeaponGrenade *pGrenadeWeapon = dynamic_cast< CWeaponGrenade* >( GetActiveWeapon() );

	if( pGrenadeWeapon && !g_fGameOver && pGrenadeWeapon->IsPrimed())
	{
		pGrenadeWeapon->DropGrenade( this );
	}
	// GRENADE DROPPING

	for( int i = 0; i < m_PlayerMagazines.Count(); i++ )
	{
		CHajWeaponMagazines *pMagazine = m_PlayerMagazines.Element(i);
		pMagazine->RemoveAll();
	}

	// PICKUP DROPPING
	if( m_hPickup.Get() )
	{
		m_hPickup.Set( NULL );

		// drop the pickup
		CHajPickup *pPickup = dynamic_cast< CHajPickup* >( m_hPickup.Get() );

		if( pPickup )
			pPickup->OwnerKilled( this, info );
	}
	// PICKUP DROPPING

	const CUtlLinkedList<CHajObjective*>& objectives = _objectiveman.GetObjectives();

	unsigned short it = objectives.Head();
	while(objectives.IsValidIndex(it))
	{
		CHajObjective* pObjective = objectives.Element(it);
		CHajPlayer* pAttacker = ToHajPlayer( info.GetAttacker() );

		if( HajGameRules() && !HajGameRules()->IsFreeplay() )
		{
			if( pObjective->IsPlayerInList( GetUserID() ) ) // killed player is in the area
			{
				pObjective->PlayerKilledOnPoint( pAttacker, this, info );
				pObjective->RemovePlayer( this );
			}
			else if( pAttacker && pObjective->IsPlayerInList( pAttacker->GetUserID() ) && !pObjective->PlayerKilledOnPoint( pAttacker, this, info ) ) // the person who killed is in a capture area
			{
				pAttacker->KillOnPoint( pObjective, this );
			}
		}

		it = objectives.Next(it);
	}

	// stop concussion
	UTIL_Concuss( this, 0, CONCUSS_STOP);

	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	
	// player is dead so make their helmet "fall off" if it hasn't already
	CreateHelmetGib( subinfo, true, true );
	
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

	BaseClass::BaseClass::Event_Killed( subinfo ); // skip HL2MP's functions

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	SetFOV( GetFOVOwner(), 0, 0.0f );
	ClearFlags();
}

//-----------------------------------------------------------------------------
// Purpose: Handle being hit
//-----------------------------------------------------------------------------
extern ConVar haj_freeplay_ff;
void CHajPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	if ( m_takedamage )
	{
		CTakeDamageInfo info = inputInfo;

		// mirror damage
		CHajPlayer *pAttacker = ToHajPlayer( info.GetAttacker() );
		CHajGameRules *pHajGameRules = HajGameRules();

		if( ( !pHajGameRules->IsFreeplay() || ( pHajGameRules->IsFreeplay() && !haj_freeplay_ff.GetBool() ) ) && pAttacker && pAttacker != this && g_pGameRules->PlayerRelationship( this, pAttacker ) == GR_TEAMMATE && friendlyfire.GetBool() && haj_ff_reflect.GetBool() )
		{
			info.ScaleDamage( haj_ff_reflect_ratio.GetFloat() );
			pAttacker->TakeDamage( info );

			DevMsg( "REFLECTING DAMAGE ONTO %s, DAMAGE %f\n", pAttacker->GetPlayerName(), info.GetDamage() );
		}

		// Prevent team damage here so blood doesn't appear
		if ( info.GetAttacker()->IsPlayer() )
		{
			if ( !g_pGameRules->FPlayerCanTakeDamage( this, info.GetAttacker() ) )
				return;
		}
		
		// scale damage based on hitbox
		SetLastHitGroup( ptr->hitgroup );
		
		switch ( ptr->hitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			info.ScaleDamage( 3.0f );
			break;
		case HITGROUP_CHEST:
			info.ScaleDamage( 1.0f );
			break;
		case HITGROUP_STOMACH:
			info.ScaleDamage( 1.0f );
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			info.ScaleDamage( 0.6f );
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			info.ScaleDamage( 0.6f );
			break;
		default:
			info.ScaleDamage( 1.0f );
			break;
		}

		// Fall off damage in a parabolic curve if over max effective range
		
		// Jed - check pointers are valid and unchain
		//CHajPlayer *pAttacker = ToHajPlayer( info.GetAttacker() );
		if ( pAttacker )
		{
			CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase *>( pAttacker->GetActiveWeapon() );
			if ( pWeapon )
			{
				float distance = ( ptr->startpos - ptr->endpos ).Length();
				float effectivedist = pWeapon->GetEfficientDistance();
				float fadefactor = pWeapon->GetBulletFadeFactor();

				// if this shot went over the max effective distance
				if( distance > effectivedist && effectivedist != 0 )
				{
					// make sure our excess distance is equal or less than the effective distance.
					float over = distance - effectivedist;
					over = clamp( over, 0, effectivedist );
					
					// return the damage factor based on a falling curve in the rance 1 - 0.
					float dmgfactor = FastCos( ( 1.57079633 / ( effectivedist * fadefactor ) ) * over );
					dmgfactor = clamp( dmgfactor, 0, 1 );

					float newdmg = info.GetDamage() * dmgfactor;
					info.SetDamage( newdmg );
				}
			}
		}

		// if you were hit in the head hard enough, lose your helmet.
		if ( ptr->hitgroup == HITGROUP_HEAD && info.GetDamage() > 50 )
		{
			// you're still wearing your helmet
			if ( m_bHasHelmet )
			{
				EmitSound( "Player.Headshot", gpGlobals->curtime + 0.05 );
				CreateHelmetGib( info, true, false );
			}
		}

		// Only show blood/spray if the damage was over 25. Helps reduce false positives
		if ( info.GetDamage() > 25 )
		{
			SpawnBlood( ptr->endpos, vecDir, BloodColor(), info.GetDamage()); // a little surface blood.
			TraceBleed( info.GetDamage(), vecDir, ptr, info.GetDamageType() );
		}

		AddMultiDamage( info, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Traps the damage even and checks if gibs should be made
//-----------------------------------------------------------------------------
int	CHajPlayer::OnTakeDamage( const CTakeDamageInfo &info )
{
	int retVal = 0;
	bool bGibbed = false;

	CTakeDamageInfo cloneinfo = info;
	CHajGameRules *pHajGamerules = HajGameRules();

	CHajPlayer *pAttacker = ToHajPlayer( info.GetAttacker() );

	// "energy beam" damage type is used for tnt. we don't want objective damage reflecting
	if( !( info.GetDamageType() & (DMG_NERVEGAS|DMG_ENERGYBEAM|DMG_ALWAYSGIB) ) && ( !pHajGamerules->IsFreeplay() || ( pHajGamerules->IsFreeplay() && !haj_freeplay_ff.GetBool() ) ) && pAttacker && pAttacker != this && g_pGameRules->PlayerRelationship( this, pAttacker ) == GR_TEAMMATE && friendlyfire.GetBool() && haj_ff_reflect.GetBool() )
	{
		cloneinfo.ScaleDamage( haj_ff_reflect_ratio.GetFloat() );
		pAttacker->TakeDamage( cloneinfo );

		DevMsg( "REFLECTING DAMAGE ONTO %s, DAMAGE %f\n", pAttacker->GetPlayerName(), cloneinfo.GetDamage() );
	}

	retVal = BaseClass::OnTakeDamage( info );
	
	// if we're hurt by a blast
	if ( info.GetDamageType() & DMG_BLAST )
	{
		// blast was > 150 in magnitude, gib.
		if ( info.GetDamage() >= 150 && m_iHealth <= 0 )
		{
			bGibbed = Event_Gibbed( info );
			retVal = bGibbed;
		}
		// otherwise if it was over 25, knock the helmet off
		else if ( info.GetDamage() >= 25 )
		{
			CreateHelmetGib( info, true, false );
		}
	}
	return retVal;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHajPlayer::UpdateOnRemove( void )
{
	// get rid of the helmet entity.
	DestroyHelmet();
	
	BaseClass::UpdateOnRemove();
}


//-----------------------------------------------------------------------------
// Purpose: create some gore 
//-----------------------------------------------------------------------------
bool CHajPlayer::CorpseGib( const CTakeDamageInfo &info )
{
	CEffectData	data;
		
	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize( data.m_vNormal );
	
	data.m_flScale = RemapVal( m_iHealth, 0, -500, 1, 3 );
	data.m_flScale = clamp( data.m_flScale, 1, 3 );

	DispatchEffect( "HumanGib", data );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Number of teammates withing a given box around you
// Notes:	Gets the number of player entities inside a given box around you
//			and checks if they are on your team or not. 
// 
//-----------------------------------------------------------------------------
#define SEARCH_DEPTH		32		// should match about half maxplayers.
#define MAX_SEARCH_HORIZ	540		// within 45 feet in either direction
#define MAX_SEARCH_VERT		540

void CHajPlayer::UpdateNearbyTeamMateCount()
{
	// drop to zero
	m_nNearbyTeamMates = 0;

	// skip if your not alive or observiing
	if ( !IsAlive() || IsObserver() )
		return;

	// array for entities
	CBaseEntity *pEnts[SEARCH_DEPTH];

	// set-up the box
	// vertical is smaller to allow for hills, etc and maybe a room above below.
	// most likel players will be in your horizontal plane.
	Vector		radius( MAX_SEARCH_HORIZ, MAX_SEARCH_HORIZ, MAX_SEARCH_VERT );
	Vector		vecSource = GetAbsOrigin();
	
	// grab only FL_CLIENT flagged entities in the box
	int numEnts = UTIL_EntitiesInBox( pEnts, SEARCH_DEPTH, vecSource-radius, vecSource+radius, FL_CLIENT );
	m_nearbyTeamMates.Purge();

	for ( int i = 0; i < numEnts; i++ )
	{
		if ( !pEnts[i] )
			continue;

		// if not a player or not on our team skip.
		if ( FClassnameIs( pEnts[i], "player" ) == false ||  GetTeamNumber() != pEnts[i]->GetTeamNumber() )
			continue;

		CBasePlayer *pPlayer = ToBasePlayer( pEnts[i] );

		if( pPlayer->IsAlive() && !pPlayer->IsObserver() )
		{
			m_nNearbyTeamMates++;
			m_nearbyTeamMates.AddToTail( pPlayer );
		}
	}

	// because you get included in the entity count, take one off for good measure
	m_nNearbyTeamMates -= 1;

	// don't check for another 5 secs.
	// helps with players transiting your zone.
	m_fNextTeamMateCheck = gpGlobals->curtime + 5.0f;
	
	return;
}

bool CHajPlayer::CanSpeak( void )
{
	/*if( IsAlive() == false || ( GetDesiredClass() == CLASS_UNASSIGNED && GetCurrentClass() == CLASS_UNASSIGNED ) )
	{
		ClientPrint( this, HUD_PRINTTALK, "#HaJ_CantSpeakWhileDead" );
		return false;
	}*/

	return true;
}

extern ConVar sv_alltalk;
ConVar haj_deadchat( "haj_deadchat", "0", FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_ARCHIVE, "Allows alive players to see dead player's chat" );
bool CHajPlayer::CanHearChatFrom( CBasePlayer *pPlayer )
{
	// this object is the player wanting to see the chat, pPlayer is the player saying something

	// filter out chat from spectators and enemies
	if( IsAlive() && pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( haj_deadchat.GetBool() )
		return true;

	// filter out chat from dead players when you're alive
	if ( IsAlive() && !pPlayer->IsAlive() ) // "this" is the player who should be able to see the chat our not, pPlayer is the player chatting
		return false;

	return true;
}

CON_COMMAND( dropmgammo, "Drop a magazine of MG ammo" )
{
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );

	if( !pPlayer ) return;

	if( pPlayer->GetCurrentClass() == CLASS_SUPPORT || pPlayer->IsEffectActive( EF_NODRAW ) || pPlayer->IsAlive() == false || pPlayer->IsObserver() )
		return;

	if( pPlayer->GetAmmoCrates( ) < 1 )
	{
		pPlayer->SendNotification( "#HaJ_NoAmmoCrates", NOTIFICATION_BASIC );
		return;
	}

	Vector vecForward, vecThrow;
	AngleVectors( pPlayer->EyeAngles(), &vecForward );

	vecThrow = ( vecForward * 256 );

	Vector vecOrigin = ( pPlayer->Weapon_ShootPosition() );

	CBaseEntity *pItem;

	if( pPlayer->GetTeamNumber() == TEAM_CWEALTH )
	{
		pItem = (CBaseEntity *)CreateEntityByName( "item_haj_alliedmgmagazine" );
	}
	else if( pPlayer->GetTeamNumber() == TEAM_AXIS )
	{
		pItem = (CBaseEntity *)CreateEntityByName( "item_haj_axismgmagazine" );
	}
	else
		return;

	if ( pItem )
	{
		pItem->SetAbsOrigin( vecOrigin );
		pItem->SetAbsAngles( pPlayer->GetAbsAngles() );
	
		pItem->ChangeTeam( pPlayer->GetTeamNumber() );
		pItem->Spawn();
		pItem->SetOwnerEntity( pPlayer );
		pItem->Activate();

		IPhysicsObject *pPhysicsObject = pItem->VPhysicsGetObject();

		if ( pPhysicsObject )
		{
			AngularImpulse impulse;
			QAngleToAngularImpulse( pPlayer->GetLocalAngularVelocity(), impulse );

			pPhysicsObject->SetVelocity( &vecThrow, &impulse );
		}

		pPlayer->TakeAmmoCrates( 1 );
	}

}

CON_COMMAND( switchtogrenade, "Switch to frag grenade (if available)" )
{
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_GetSlot( 2 ); // frag slot

	if( pWeapon )
	{
		pPlayer->Weapon_Switch( pWeapon );
	}
}

CON_COMMAND( vocalize, "Voice command" )
{
	if ( engine->Cmd_Argc() != 2 )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	char const *soundName = engine->Cmd_Argv( 1 );

	pPlayer->DoVoiceCommand( soundName );
}

void CHajPlayer::DoVoiceCommand( const char *strCommand )
{
	if( gpGlobals->curtime < m_flNextVoiceCommand || !IsAlive() || IsObserver() || !CHajTeam::IsCombatTeam( GetTeamNumber() ) )
		return;

	CHajGameRules *pGameRules = HajGameRules();

	char strVoice[150] = "";

	if( pGameRules && m_iAccent > -1 )
	{
		DevMsg( "using random accent %d, %s\n", m_iAccent, pGameRules->m_Accents[GetTeamNumber()].Element( m_iAccent ) );
		Q_snprintf( strVoice, sizeof( strVoice), "Voice.%s%s", pGameRules->m_Accents[GetTeamNumber()].Element( m_iAccent ), strCommand );
	}
	else
	{
		// DEFAULT HANDLING
		if( GetTeamNumber() == TEAM_CWEALTH )
		{
			if( pGameRules && pGameRules->GetCommonwealthNation() == NATION_CWEALTH_POLAND )
				Q_snprintf( strVoice, sizeof( strVoice ), "Voice.Pol%s", strCommand );
			else if( pGameRules && pGameRules->GetCommonwealthNation() == NATION_CWEALTH_CANADA )
				Q_snprintf( strVoice, sizeof( strVoice ), "Voice.Can%s", strCommand );
			else
				Q_snprintf( strVoice, sizeof( strVoice ), "Voice.Brit%s", strCommand );
		}
		else
			Q_snprintf( strVoice, sizeof( strVoice ), "Voice.Ger%s", strCommand );
	}

	EmitSound( strVoice );

	float flSoundDuration = GetSoundDuration( strVoice, STRING( GetModelName() ) );
	m_flNextVoiceCommand = gpGlobals->curtime + flSoundDuration;

	if( flSoundDuration )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "player_voice_command" );
		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetInt( "teamid", GetTeamNumber() );
			event->SetInt( "entindex", entindex() );
			event->SetString( "vcommand", strCommand );
			event->SetString( "soundref", strVoice );

			event->SetInt(	"priority", 7 );	// HLTV event priority, not transmitted

			gameeventmanager->FireEvent( event );
		}
	}

}


const char* CHajPlayer::GetVoiceString( int iMenu, int iSelection )
{
	DevMsg( "Getting voice string for %d menu, %d slot\n", iMenu, iSelection );

	if( iMenu == 1 )
	{
		switch( iSelection )
		{
		case 1: // go go go
			return "Go";
			break;

		case 2: // flank left
			return "FlankLeft";
			break;

		case 3: // flank right
			return "FlankRight";
			break;

		case 4: // fall back
			return "FallBack";
			break;

		case 5: // take the objective
			return "Obj";
			break;

		case 6: // follow me
			return "FollowMe";
			break;

		case 7: // Take Cover
			return "TakeCover";
			break;

		case 8: // Hold This Position
			return "Hold";
			break;

		case 9: // Incoming Grenade
		default:
			return "Grenade";
		}
	}
	else
	{
		switch( iSelection )
		{
		case 1: // Grenade Out!
			return "GrenadeOut";
			break;

		case 2: // Sniper
			return "Snip";
			break;

		case 3: // MG Ahead
			return "MGAhead";
			break;

		case 4: // Incoming Fire from Left
			return "IncomingLeft";
			break;

		case 5: // Incoming Fire from Right
			return "IncomingRight";
			break;

		case 6: // Smoke grenade needed
			return "Smoke";
			break;

		case 7: // I need ammo
			return "Ammo";
			break;

		case 8: // Enemy Spotted
			return "EnemySpotted";
			break;

		case 9: // Covering Fire
		default:
			return "CoverFire";
		}
	}

	return "Go"; // default, should never get here
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *cmd - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayer::ClientCommand(const char *cmd)
{
	return BaseClass::ClientCommand( cmd );
}

void CHajPlayer::ShowClassMenu( void )
{
	ShowViewPortPanel( PANEL_HAJCLASS );

	/*if( GetTeamNumber() == TEAM_CWEALTH )
	{
		ShowViewPortPanel( PANEL_HAJCLASS_CWEALTH );
	}
	else if( GetTeamNumber() == TEAM_AXIS )
	{
		ShowViewPortPanel( PANEL_HAJCLASS_AXIS );
	}*/
}

extern ConVar haj_allowunbalancedteams;
bool CHajPlayer::HandleCommand_JoinTeam( int team )
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

	CTeam *pAxis = g_Teams[TEAM_AXIS];
	CTeam *pCWealth = g_Teams[TEAM_CWEALTH];

	bool bSwitchingFromCombat = CHajTeam::IsCombatTeam(GetTeamNumber());

	random->SetSeed( gpGlobals->curtime * GetUserID() );

	// Auto-assign
	if ( team == TEAM_AUTO || team == 0 )	// Auto assign
	{
		if ( pAxis == NULL || pCWealth == NULL )
		{
			team = random->RandomInt( TEAM_CWEALTH, TEAM_AXIS );
		}
		else
		{
			if ( pAxis->GetNumPlayers() - pCWealth->GetNumPlayers() > 0 )
			{
				team = TEAM_CWEALTH;
			}
			else if ( pCWealth->GetNumPlayers() - pAxis->GetNumPlayers() > 0 )
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
			CommitSuicide();

			m_iClass = CLASS_UNASSIGNED;
			m_iCurrentClass = m_iClass;
		}

		ChangeTeam( TEAM_SPECTATOR );
		return true;		
	}

	if( !haj_allowunbalancedteams.GetBool() )
	{
		if( (team == TEAM_CWEALTH && pCWealth->GetNumPlayers() > pAxis->GetNumPlayers() + ( bSwitchingFromCombat ? -1 : 0 ) )
			|| (team == TEAM_AXIS && pAxis->GetNumPlayers() > pCWealth->GetNumPlayers() + ( bSwitchingFromCombat ? -1 : 0 ) ) )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#HaJ_TeamIsFull" );
			engine->ClientCommand( edict(), "chooseteam" );

			return false;
		}
	}

	// we're swapping team so clear our current class;
	m_iClass = CLASS_UNASSIGNED;
	m_iCurrentClass = m_iClass;
	SetDesiredClass( CLASS_UNASSIGNED );
	m_flNextClassChangeTime = gpGlobals->curtime;

	m_lifeState = LIFE_DEAD;

	ChangeTeam( team );
	ShowClassMenu();

	return true;
}

const char* CHajPlayer::GetPlayerLocation( bool bTeamChat )
{
	if( bTeamChat )
	{
		Q_snprintf( m_strLocationTeamChat, sizeof( m_strLocationTeamChat ), "(TEAM @ %s)", m_strLocation );
		return m_strLocationTeamChat;
	}

	return GetPlayerLocation();
}


void CHajPlayer::CommitSuicide()
{
	if( !g_fGameOver )
	{
		BaseClass::CommitSuicide();
	}
}

void CHajPlayer::SetInBombZone( bool b, CBombZone *pBombZone /*= NULL */ )
{
	m_bInBombZone = b;
	
	if( pBombZone )
		m_hBombZone.Set( pBombZone );
}

void CHajPlayer::SendNotification( const char* szLocalise, int iNotificationType, int iCE /*= 0*/, CBaseEntity *pEnt /*= NULL*/, const char* szEntName /*= NULL */ )
{
	// filter
	CSingleUserRecipientFilter userfilter( (CBasePlayer*)this );
	userfilter.MakeReliable();

	// send
	UserMessageBegin( userfilter, "HajPlayerNotification" );
		WRITE_STRING( szLocalise );
		WRITE_SHORT( iNotificationType );

		if( iNotificationType > 0 )
		{
			WRITE_SHORT( pEnt ? pEnt->entindex() : 0 );
			WRITE_STRING( szEntName );
		}

		WRITE_SHORT( iCE );
	MessageEnd();
}

void CHajPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData /*= 0 */ )
{
	m_pHajAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

void CHajPlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	switch( playerAnim )
	{
		case PLAYER_JUMP:
			DoAnimationEvent( PLAYERANIMEVENT_JUMP );
			break;

		default:
			; // dunno
	}
}

/*
CON_COMMAND( voice_grenade, "Grenade!" ) 
{ 
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	pPlayer->DoVoiceCommand("Grenade"); 
 }

CON_COMMAND( voice_go, "Go Go Go!" ) 
{ 
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	pPlayer->DoVoiceCommand("Go"); 
 }

CON_COMMAND( voice_takeobj, "Take The Objective!" ) 
{ 
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	pPlayer->DoVoiceCommand("Obj"); 
 }

CON_COMMAND( voice_sniper, "Sniper!" ) 
{ 
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	pPlayer->DoVoiceCommand("Snip"); 
 }

CON_COMMAND( voice_hold, "Hold!" ) 
{ 
	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );
	pPlayer->DoVoiceCommand("Hold"); 
 }
*/

extern bool g_fGameOver;
CON_COMMAND( vo_menu_1, "Voice command menu #1" )
{
	if( g_fGameOver ) return;

	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );

	if( pPlayer && pPlayer->IsAlive() && !pPlayer->IsObserver() && pPlayer->CanUseVoiceCommands() )
	{
		pPlayer->m_iOpenVoiceMenu = 1;

		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable();
		
		UserMessageBegin( user, "ShowMenu" );
			WRITE_SHORT( (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) | (1<<10)  );
			WRITE_CHAR( -1 );
			WRITE_BYTE( false );
			WRITE_STRING( "#VoiceComA" );
		MessageEnd();
	}
}

CON_COMMAND( vo_menu_2, "Voice command menu #2" )
{
	if( g_fGameOver ) return;

	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );

	if( pPlayer && pPlayer->IsAlive() && !pPlayer->IsObserver() && pPlayer->CanUseVoiceCommands() )
	{
		pPlayer->m_iOpenVoiceMenu = 2;

		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "ShowMenu" );
		WRITE_SHORT( (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) | (1<<10) );
		WRITE_CHAR( -1 );
		WRITE_BYTE( false );
		WRITE_STRING( "#VoiceComB" );
		MessageEnd();
	}
}

CON_COMMAND( chooseclass, "Show the class menu" )
{
	if( g_fGameOver ) return;

	CHajPlayer *pPlayer = ToHajPlayer( UTIL_GetCommandClient() );

	if( pPlayer )
		pPlayer->ShowClassMenu();
}

/*
CON_COMMAND( vo_menu_2, "Voice command menu #2" )
{
	CHudMenu *pHudMenu = GET_HUDELEMENT( CHudMenu );

	if( pHudMenu )
	{
		pHudMenu->ShowMenu( "HaJ_VoiceCom_B", ( 1 << 0  );
	}
}

CON_COMMAND( vo_menu_unlocalised_test, "Voice command menu #2" )
{
	CHudMenu *pHudMenu = GET_HUDELEMENT( CHudMenu );

	if( pHudMenu )
	{
		pHudMenu->ShowMenu( "Voice Commands\n\n1. Go Go Go\n2. Flank Left\n3. Flank Right\n4. Fall Back\n5. Take The Objective\n6. Follow Me\n7. Take Cover\n8. Hold This Position\n9. Incoming Grenade\n\n0. Close\n", 9 );
	}
}*/
