//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ base machine gun class
// Notes:    This replaces the default HL2MP machine gun class and adds
//           the feaure of magazine based ammo management.
//
//           The magazines/ammo is handled locally in the weapon rather
//			 than taking ammo from the player "buckets".
//			
//			 Most of these functions in the class simply override the
//			 defaults to get around the bucket code and use the magazine
//			 stuff instead.
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "datacache/imdlcache.h"	

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "haj_weapon_machinegun.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( HAJMachineGun, DT_HAJMachineGun )

BEGIN_NETWORK_TABLE( CHAJMachineGun, DT_HAJMachineGun )
#ifdef CLIENT_DLL
	RecvPropArray3( RECVINFO_ARRAY( m_iAmmoInClip ), RecvPropInt( RECVINFO( m_iAmmoInClip[0] )) ),
#else
	SendPropArray3( SENDINFO_ARRAY3( m_iAmmoInClip ), SendPropInt( SENDINFO_ARRAY( m_iAmmoInClip ), 10, SPROP_UNSIGNED ) ),
#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CHAJMachineGun )
END_PREDICTION_DATA()

//=========================================================
//	>> CHLSelectFireMachineGun
//=========================================================
BEGIN_DATADESC( CHAJMachineGun )

	DEFINE_FIELD( m_nShotsFired,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),
	DEFINE_AUTO_ARRAY( m_iAmmoInClip, FIELD_INTEGER ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Construtor
//-----------------------------------------------------------------------------
CHAJMachineGun::CHAJMachineGun( void )
{
	// zero out the clips at construction time
	m_iCurrentClip = -1;
	m_iClipsLeft = 0;

	for ( int i = 0; i < MAX_CLIPS_PER_WEAPON; i++)
		m_iAmmoInClip.Set( i, 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Default cone of fire. Should be defined locally for each weapon.
//-----------------------------------------------------------------------------
const Vector &CHAJMachineGun::GetBulletSpread( void )
{
	static Vector cone = VECTOR_CONE_3DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: Handles firing the weapon
//-----------------------------------------------------------------------------
void CHAJMachineGun::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;
	
	// Auto reload or play the out of ammo sound
	if ( ( UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}
		return;

	}

	m_nShotsFired++;

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	// FIXME: only called once, will miss multiple sound events per frame if needed
	// FIXME: m_flNextPrimaryAttack is always in the past, it's not clear what'll happen with sounds
	WeaponSound(SINGLE, m_flNextPrimaryAttack);
	// Msg("%.3f\n", m_flNextPrimaryAttack.Get() );

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if ( UsesClipsForAmmo1() )
	{
		if ( iBulletsToFire > m_iClip1 )
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}

	CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer( pPlayer );

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pHL2MPPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	info.m_vecSpread = pHL2MPPlayer->GetAttackSpread( this );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 5; // *HAJ 020* - Jed 1:5 ball/tracer mix
	FireBullets( info );

	//Factor in the view kick
	AddViewKick();
	
	// check if the current clip is empty and the next one to use is empty too
	if (!m_iClip1 && m_iCurrentClip != -1 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	SendWeaponAnim( GetPrimaryAttackActivity() );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CHAJMachineGun::FireBullets( const FireBulletsInfo_t &info )
{
	if(CBasePlayer *pPlayer = ToBasePlayer ( GetOwner() ) )
	{
		pPlayer->FireBullets(info);
	}
}

//-----------------------------------------------------------------------------
// Purpose: This kicks the view around when firing a little.
// Note: This is NOT recoil.
//-----------------------------------------------------------------------------
void CHAJMachineGun::DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime )
{
	#define	KICK_MIN_X			0.2f	//Degrees
	#define	KICK_MIN_Y			0.2f	//Degrees
	#define	KICK_MIN_Z			0.1f	//Degrees

	QAngle vecScratch;
	int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	
	//Find how far into our accuracy degradation we are
	float duration	= ( fireDurationTime > slideLimitTime ) ? slideLimitTime : fireDurationTime;
	float kickPerc = duration / slideLimitTime;

	// do this to get a hard discontinuity, clear out anything under 10 degrees punch
	pPlayer->ViewPunchReset( 10 );

	//Apply this to the view angles as well
	vecScratch.x = -( KICK_MIN_X + ( maxVerticleKickAngle * kickPerc ) );
	vecScratch.y = -( KICK_MIN_Y + ( maxVerticleKickAngle * kickPerc ) ) / 3;
	vecScratch.z = KICK_MIN_Z + ( maxVerticleKickAngle * kickPerc ) / 8;

	RandomSeed( iSeed );

	//Wibble left and right
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.y *= -1;

	iSeed++;

	//Wobble up and down
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.z *= -1;

	//Clip this to our desired min/max
	UTIL_ClipPunchAngleOffset( vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle( 24.0f, 3.0f, 1.0f ) );

	//Add it to the view punch
	// NOTE: 0.5 is just tuned to match the old effect before the punch became simulated
	pPlayer->ViewPunch( vecScratch * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset our shots fired
//-----------------------------------------------------------------------------
bool CHAJMachineGun::Deploy( void )
{
	m_nShotsFired = 0;

	return BaseClass::Deploy();
}

// *HAJ 020* - Jed
// Adds view "bob" to machine gun derived weapons
#if defined( CLIENT_DLL )

#define	HL2_BOB_CYCLE_MIN	1.0f
#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB			0.002f
#define	HL2_BOB_UP		0.5f

extern float	g_lateralBob;
extern float	g_verticalBob;

static ConVar	cl_bobcycle( "cl_bobcycle","0.8" );
static ConVar	cl_bob( "cl_bob","0.002" );
static ConVar	cl_bobup( "cl_bobup","0.5" );

// Register these cvars if needed for easy tweaking
static ConVar	v_iyaw_cycle( "v_iyaw_cycle", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iroll_cycle( "v_iroll_cycle", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_ipitch_cycle( "v_ipitch_cycle", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iyaw_level( "v_iyaw_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iroll_level( "v_iroll_level", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_ipitch_level( "v_ipitch_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CHAJMachineGun::CalcViewmodelBob( void )
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;
	
	CBasePlayer *player = ToBasePlayer( GetOwner() );
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ( ( !gpGlobals->frametime ) || ( player == NULL ) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	speed = clamp( speed, -320, 320 );

	float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );
	
	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX)*HL2_BOB_CYCLE_MAX;
	cycle /= HL2_BOB_CYCLE_MAX;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}
	
	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX*2)*HL2_BOB_CYCLE_MAX*2;
	cycle /= HL2_BOB_CYCLE_MAX*2;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );
	
	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CHAJMachineGun::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * 0.1f, forward, origin );
	
	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;
	
	// bob the angles
	angles[ ROLL ]	+= g_verticalBob * 0.5f;
	angles[ PITCH ]	-= g_verticalBob * 0.4f;

	angles[ YAW ]	-= g_lateralBob  * 0.3f;

	VectorMA( origin, g_lateralBob * 0.8f, right, origin );
}

#else

// Server stubs
float CHAJMachineGun::CalcViewmodelBob( void )
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CHAJMachineGun::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CHAJMachineGun::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if (m_flNextSoundTime < gpGlobals->curtime)
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJMachineGun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	// Debounce the recoiling counter
	if ( ( pOwner->m_nButtons & IN_ATTACK ) == false )
	{
		m_nShotsFired = 0;
	}

	// temporary stuff to show the ammo status.

/*
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	int i;
	engine->Con_NPrintf( 0, "Main Clip: %d", m_iClip1 );

	for (i = 0; i < MAX_CLIPS_PER_WEAPON; i++)
		engine->Con_NPrintf( i+1, "Clip %d: %d", i, m_iAmmoInClip[i] );
	engine->Con_NPrintf( i+1, "InReload: %d", m_bInReload ? 1:0 );
#endif
*/
	BaseClass::ItemPostFrame();

}

//-----------------------------------------------------------------------------
// Purpose: Goes through all the mags and selects the one with the most ammo.
// NOTE: Returns the index to the clip to use, or -1 if theres no ammo left.
//-----------------------------------------------------------------------------
bool CHAJMachineGun::GetNextMagazine()
{
	int iAmmoLeft = 0;
	int iAmmoInClip = 0;
	int iClipToUse = -1;

	// check all clips
	for ( int i = 0; i < MAX_CLIPS_PER_WEAPON; i++)
	{
		iAmmoInClip = m_iAmmoInClip.Get( i );
		if ( iAmmoInClip > iAmmoLeft )
		{
			iAmmoLeft = iAmmoInClip;
			iClipToUse = i;
		}
	}
	m_iCurrentClip = iClipToUse;
	
	return m_iCurrentClip != -1 ? true: false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns how many of our spare mags have ammo in them
//-----------------------------------------------------------------------------
int CHAJMachineGun::MagsLeft( void )
{
	int mags = 0;

	for ( int i = 0; i < MAX_CLIPS_PER_WEAPON; i++)
		if ( m_iAmmoInClip.Get( i ) > 0 )
			mags++;

	return mags;
}

//-----------------------------------------------------------------------------
// Purpose: Override the default reload function
//-----------------------------------------------------------------------------
bool CHAJMachineGun::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if ( !GetNextMagazine() )
		return false;

	// If the current clip isn't full and we have ammo, reload.
	if ( !( (iClipSize1 - m_iClip1) != 0 && (m_iCurrentClip != -1) ) )
			return false;

#ifdef CLIENT_DLL
	WeaponSound( RELOAD ); // Play reload sound
#endif
	
	SendWeaponAnim( iActivity );

	// Play the player's reload animation
	if ( pOwner->IsPlayer() )
		( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_RELOAD );

	MDLCACHE_CRITICAL_SECTION();
	float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
	pOwner->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	m_bInReload = true;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Reload has finished. Actuall updating of ammo happens here.
// TODO: Need to stop this activating all the time when autoreload is OFF!!!!
//-----------------------------------------------------------------------------
void CHAJMachineGun::FinishReload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner)
	{
		// we have ammo in another clip
		if ( GetNextMagazine() )
		{
			// how much ammo do we have and how much in the new mag?
			int iCurrentAmmo = m_iClip1.Get();
			int iFromClip = m_iAmmoInClip.Get( m_iCurrentClip );
			
			// take it from the spare to our current mag
			m_iClip1.Set( iFromClip );
			m_iAmmoInClip.Set( m_iCurrentClip, iCurrentAmmo );
				
			// update the spare mag counter
			m_iClipsLeft = MagsLeft();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CHAJMachineGun::HasPrimaryAmmo( void )
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if ( UsesClipsForAmmo1() && m_iClip1 > 0 )
			return true;

	// if our current weapon mag is empty, check the spares
	return GetNextMagazine();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJMachineGun::CheckReload( void )
{
	if ( m_bReloadsSingly )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( !pOwner )
			return;

		if ((m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			if ( pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0 )
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if ( !GetNextMagazine() )
			{
				FinishReload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				m_flNextPrimaryAttack	= gpGlobals->curtime;
				m_flNextSecondaryAttack = gpGlobals->curtime;
				return;
			}
		}
	}
	else
	{
		if ( (m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			FinishReload();
			m_flNextPrimaryAttack	= gpGlobals->curtime;
			m_flNextSecondaryAttack = gpGlobals->curtime;
			m_bInReload = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fills our spare mags
//-----------------------------------------------------------------------------
void CHAJMachineGun::PopulateMagazines ( int m )
{
	// don't go over max permissible magazines
	if ( m > MAX_CLIPS_PER_WEAPON )
		m = MAX_CLIPS_PER_WEAPON;

	// get default magazine size
	int c = GetMaxClip1();

	// fill magazines (they should of previously been emptied)
	for ( int i = 0; i < m; i++)
		m_iAmmoInClip.Set( i, c );

	// update our spare mag counter
	m_iClipsLeft = MagsLeft();
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------

static ConVar haj_autoreload( "haj_autoreload", "1", FCVAR_ARCHIVE, "Enable/disable auto reloading of weapon." );
static ConVar haj_autoswitch( "haj_autoswitch", "1", FCVAR_ARCHIVE, "Enable/disable auto switching to next best weapon" );

bool CHAJMachineGun::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		// weapon isn't useable, switch.
		if ( haj_autoswitch.GetBool() == true  && g_pGameRules->SwitchToNextBestWeapon( pOwner, this ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		// control auto-reloads via console var.
		if ( UsesClipsForAmmo1() && 
			 ( m_iClip1 == 0 ) && 
			 haj_autoreload.GetBool() == true &&
			 m_flNextPrimaryAttack < gpGlobals->curtime && 
			 m_flNextSecondaryAttack < gpGlobals->curtime )
		{
				return  Reload();
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns COF angle vector based on int.
//-----------------------------------------------------------------------------
Vector CHAJMachineGun::GetCoFVector( unsigned int coneangle )
{
	switch ( coneangle )
	{
	case 1:
		return VECTOR_CONE_1DEGREES;
		break;
	case 2:
		return VECTOR_CONE_2DEGREES;
		break;
	case 3:
		return VECTOR_CONE_3DEGREES;
		break;
	case 4:
		return VECTOR_CONE_4DEGREES;
		break;
	case 5:
		return VECTOR_CONE_5DEGREES;
		break;
	case 6:
		return VECTOR_CONE_6DEGREES;
		break;
	case 7:
		return VECTOR_CONE_7DEGREES;
		break;
	case 8:
		return VECTOR_CONE_8DEGREES;
		break;
	case 9:
		return VECTOR_CONE_9DEGREES;
		break;
	case 10:
		return VECTOR_CONE_10DEGREES;
		break;
	case 15:
		return VECTOR_CONE_15DEGREES;
		break;
	case 20:
	default:
		return VECTOR_CONE_20DEGREES;
		break;
	}
}