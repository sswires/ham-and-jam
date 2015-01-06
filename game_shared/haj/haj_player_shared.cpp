//========= Copyright © 2008, Ham and Jam. ==============================//
// Purpose: Shared player functions
//
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"

#include "decals.h"
#include "effect_dispatch_data.h"
#include "model_types.h"
#include "func_vaulthint.h"
#include "gamestringpool.h"
#include "ammodef.h"
#include "takedamageinfo.h"
#include "shot_manipulator.h"
#include "ai_debug_shared.h"
#include "mapentities_shared.h"
#include "debugoverlay_shared.h"
#include "coordsize.h"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
#else
	#include "te_effect_dispatch.h"
	#include "soundent.h"
	#include "iservervehicle.h"
	#include "player_pickup.h"
	#include "waterbullet.h"
	#include "ilagcompensationmanager.h"
	#include "baseentity.h"
#ifdef HL2MP
	#include "te_hl2mp_shotgun_shot.h"
#endif

#endif

#ifdef CLIENT_DLL
	#include "haj_player_c.h"
	#include "prediction.h"
#else
	#include "haj_player.h"
#endif

#include "haj_gamerules.h"
#include "weapon_hl2mpbase.h"
#include "haj_weapon_base.h"

extern ConVar sv_showimpacts;
extern ConVar sv_impactsfadetime;
extern ConVar ai_debug_shoot_positions;

// define red as our blood colour
int CHajPlayer::BloodColor()
{
	return BLOOD_COLOR_RED;
}


extern ConVar haj_nocollideteam;
bool CHajPlayer::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	// DISALLOW COLLISIONS BETWEEN TEAM MATES
	if( ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup == COLLISION_GROUP_PLAYER ) && haj_nocollideteam.GetBool() )
	{
		switch( GetTeamNumber() )
		{
		case TEAM_CWEALTH:
			if( contentsMask & CONTENTS_TEAM1 )
				return false;

			break;

		case TEAM_AXIS:
			if( contentsMask & CONTENTS_TEAM2 )
				return false;

			break;
		}
	}

	return CBaseEntity::ShouldCollide( collisionGroup, contentsMask );
}

#define MIN_GROUND_VAULT_HEIGHT 32.0f
#define MIN_AIR_VAULT_HEIGHT 18.0f
#define MG_VIEW_F_DISP 2.0f

bool CHajPlayer::CanVault()
{

	if( m_Local.m_bProned || m_Local.m_bProning || m_Local.m_bDucked || m_Local.m_bDucking )
		return false;

	if( m_Local.m_hVaultHint != NULL )
	{
		return m_Local.m_hVaultHint.Get()->CanPlayerVault( this );
	}

	return false; //Not even solid at waist
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/

void CHajPlayer::FireBullets( FireBulletsInfo_t &info, bool doLagComp )
{
#ifdef GAME_DLL
	// Move other players back to history positions based on local player's lag
	if( doLagComp )
		lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );
#endif

	if( info.m_iDamage < 1 )
	{
		CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

		if ( pWeapon )
		{
			info.m_iPlayerDamage = info.m_iDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
		}
	}

#ifdef GAME_DLL
	NoteWeaponFired();
#endif

	// *HAJ 020* - Jed - record time last shot was fired.
	SetLastShotTime( gpGlobals->curtime );


	static int	tracerCount;
	trace_t		tr;
	CAmmoDef*	pAmmoDef	= GetAmmoDef();
	int			nDamageType	= pAmmoDef->DamageType(info.m_iAmmoType);
	int			nAmmoFlags	= pAmmoDef->Flags(info.m_iAmmoType);
	
	bool bDoServerEffects = true;

#if defined( GAME_DLL )
	bDoServerEffects = false;
#endif

#if defined ( _XBOX ) && defined( GAME_DLL )
	if( IsPlayer() )
	{
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(this);

		int rumbleEffect = pGetActiveWeapon()->GetRumbleEffect();

		if( rumbleEffect != RUMBLE_INVALID )
		{
			if( rumbleEffect == RUMBLE_SHOTGUN_SINGLE )
			{
				if( info.m_iShots == 12 )
				{
					// Upgrade to double barrel rumble effect
					rumbleEffect = RUMBLE_SHOTGUN_DOUBLE;
				}
			}

			pRumbleEffect( rumbleEffect, 0, RUMBLE_FLAG_RESTART );
		}
	}
#endif//_XBOX

	int iPlayerDamage = info.m_iPlayerDamage;
	if ( iPlayerDamage == 0 )
	{
		if ( nAmmoFlags & AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER )
		{
			iPlayerDamage = pAmmoDef->PlrDamage( info.m_iAmmoType );
		}
	}

	// the default attacker is ourselves
	CBaseEntity *pAttacker = info.m_pAttacker ? info.m_pAttacker : this;

	// Make sure we don't have a dangling damage target from a recursive call
	if ( g_MultiDamage.GetTarget() != NULL )
	{
		ApplyMultiDamage();
	}
	  
	ClearMultiDamage();
	g_MultiDamage.SetDamageType( nDamageType | DMG_NEVERGIB );

	Vector vecDir;
	Vector vecEnd;
	Vector vecFinalDir;	// bullet's final direction can be changed by passing through a portal
	
	CTraceFilterSkipTwoEntities traceFilter( this, info.m_pAdditionalIgnoreEnt, COLLISION_GROUP_NONE );

	bool bUnderwaterBullets = true;
	bool bStartedInWater = false;
	if ( bUnderwaterBullets )
	{
		bStartedInWater = ( enginetrace->GetPointContents( info.m_vecSrc ) & (CONTENTS_WATER|CONTENTS_SLIME) ) != 0;
	}

	// Prediction is only usable on players
	int iSeed = 0;
	if ( IsPlayer() )
	{
		iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	}

#if defined( HL2MP ) && defined( GAME_DLL )
	int iEffectSeed = iSeed;
#endif
	//-----------------------------------------------------
	// Set up our shot manipulator.
	//-----------------------------------------------------
	CShotManipulator Manipulator( info.m_vecDirShooting );

	bool bDoImpacts = false;
	bool bDoTracers = false;
	
	for (int iShot = 0; iShot < info.m_iShots; iShot++)
	{
		bool bHitWater = false;
		bool bHitGlass = false;

		// Prediction is only usable on players
		if ( IsPlayer() )
		{
			RandomSeed( iSeed );	// init random system with this seed
		}

		// If we're firing multiple shots, and the first shot has to be bang on target, ignore spread
		if ( iShot == 0 && info.m_iShots > 1 && (info.m_nFlags & FIRE_BULLETS_FIRST_SHOT_ACCURATE) )
		{
			vecDir = Manipulator.GetShotDirection();
		}
		else
		{

			// Don't run the biasing code for the player at the moment.
			vecDir = Manipulator.ApplySpread( info.m_vecSpread );
		}

		vecEnd = info.m_vecSrc + vecDir * info.m_flDistance;

		if( IsPlayer() && info.m_iShots > 1 && iShot % 2 )
		{
			// Half of the shotgun pellets are hulls that make it easier to hit targets with the shotgun.
			AI_TraceHull( info.m_vecSrc, vecEnd, Vector( -3, -3, -3 ), Vector( 3, 3, 3 ), MASK_SHOT, &traceFilter, &tr );
		}
		else
		{
			AI_TraceLine(info.m_vecSrc, vecEnd, MASK_SHOT, &traceFilter, &tr);
		}

		vecFinalDir = tr.endpos - tr.startpos;
		VectorNormalize( vecFinalDir );

#ifdef GAME_DLL
		if ( ai_debug_shoot_positions.GetBool() )
			NDebugOverlay::Line(info.m_vecSrc, vecEnd, 255, 255, 255, false, .1 );
#endif

		if ( bStartedInWater )
		{
#ifdef GAME_DLL
			CreateBubbleTrailTracer( info.m_vecSrc, tr.endpos, vecFinalDir );
#endif
			bHitWater = true;
		}

		// Now hit all triggers along the ray that respond to shots...
		// Clip the ray to the first collided solid returned from traceline
		CTakeDamageInfo triggerInfo( pAttacker, pAttacker, info.m_iDamage, nDamageType );
		CalculateBulletDamageForce( &triggerInfo, info.m_iAmmoType, vecFinalDir, tr.endpos );
		triggerInfo.ScaleDamageForce( info.m_flDamageForceScale );
		triggerInfo.SetAmmoType( info.m_iAmmoType );
#ifdef GAME_DLL
		TraceAttackToTriggers( triggerInfo, tr.startpos, tr.endpos, vecFinalDir );
#endif

		// Make sure given a valid bullet type
		if (info.m_iAmmoType == -1)
		{
			DevMsg("ERROR: Undefined ammo type!\n");
			return;
		}

		Vector vecTracerDest = tr.endpos;

		// do damage, paint decals
		if (tr.fraction != 1.0)
		{
#ifdef GAME_DLL
			// For shots that don't need persistance
			int soundEntChannel = ( info.m_nFlags&FIRE_BULLETS_TEMPORARY_DANGER_SOUND ) ? SOUNDENT_CHANNEL_BULLET_IMPACT : SOUNDENT_CHANNEL_UNSPECIFIED;

			CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, tr.endpos, 200, 0.5, this, soundEntChannel );
#endif

			// See if the bullet ended up underwater + started out of the water
			if ( !bHitWater && ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) ) )
			{
				bHitWater = HandleShotImpactingWater( info, vecEnd, &traceFilter, &vecTracerDest );
			}

			float flActualDamage = info.m_iDamage;

			// If we hit a player, and we have player damage specified, use that instead
			// Adrian: Make sure to use the currect value if we hit a vehicle the player is currently driving.
			if ( iPlayerDamage )
			{
				if ( tr.m_pEnt->IsPlayer() )
				{
					flActualDamage = iPlayerDamage;
				}
#ifdef GAME_DLL
				else if ( tr.m_pEnt->GetServerVehicle() )
				{
					if ( tr.m_pEnt->GetServerVehicle()->GetPassenger() && tr.m_pEnt->GetServerVehicle()->GetPassenger()->IsPlayer() )
					{
						flActualDamage = iPlayerDamage;
					}
				}
#endif
			}


// HAJ 020 - Jed
// If a shot hits the arms, check to see if it would of passed through and
// hit another body part. If so, register the hit there instead.
#ifdef GAME_DLL

			// check if we're a player and hit the left or right arm hitboxes
			if ( tr.m_pEnt->IsPlayer() && ( tr.hitgroup == HITGROUP_LEFTARM || tr.hitgroup == HITGROUP_RIGHTARM ) )
			{
				// grab a hook to our victim
				CBasePlayer *pBasePlayer = ToBasePlayer( tr.m_pEnt );
				
				if ( pBasePlayer )
				{
					// set the no arm hit boxes
					pBasePlayer->SetHitboxSet( 1 );
					
					// redo the trace
					trace_t newtr;					
					AI_TraceLine( info.m_vecSrc, vecEnd, MASK_SHOT, &traceFilter, &newtr );
					
					// compare if the shot hit the same player
					if ( tr.m_pEnt == newtr.m_pEnt )
					{
						// use our new non-arm hitbox data instead.
						Assert( tr.hitgroup != newtr.hitgroup ); // If we hit this, hitbox sets are broken
						tr = newtr;
					}
					
					// revert back to the arm hitbox set.
					pBasePlayer->SetHitboxSet( 0 );
				}
			}
#endif

			int nActualDamageType = nDamageType;
			if ( flActualDamage == 0.0 )
			{
				flActualDamage = g_pGameRules->GetAmmoDamage( pAttacker, tr.m_pEnt, info.m_iAmmoType );
			}
			else
			{
				nActualDamageType = nDamageType | ((flActualDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB );
			}

			if ( !bHitWater || ((info.m_nFlags & FIRE_BULLETS_DONT_HIT_UNDERWATER) == 0) )
			{
				// Damage specified by function parameter
				CTakeDamageInfo dmgInfo( this, pAttacker, flActualDamage, nActualDamageType );
				CalculateBulletDamageForce( &dmgInfo, info.m_iAmmoType, vecFinalDir, tr.endpos );
				dmgInfo.ScaleDamageForce( info.m_flDamageForceScale );
				dmgInfo.SetAmmoType( info.m_iAmmoType );
				tr.m_pEnt->DispatchTraceAttack( dmgInfo, vecFinalDir, &tr );
			
				if ( bStartedInWater || !bHitWater || (info.m_nFlags & FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS) )
				{
					if ( bDoServerEffects == true )
					{
						DoImpactEffect( tr, nDamageType );
					}
					else
					{
						bDoImpacts = true;
					}
				}
				else
				{
					// We may not impact, but we DO need to affect ragdolls on the client
					CEffectData data;
					data.m_vStart = tr.startpos;
					data.m_vOrigin = tr.endpos;
					data.m_nDamageType = nDamageType;
					
					DispatchEffect( "RagdollImpact", data );
				}
	
#ifdef GAME_DLL
				if ( nAmmoFlags & AMMO_FORCE_DROP_IF_CARRIED )
				{
					// Make sure if the player is holding this, he drops it
					Pickup_ForcePlayerToDropThisObject( tr.m_pEnt );		
				}
#endif
			}
		}

		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );

		// See if we hit glass
		if ( tr.m_pEnt != NULL )
		{
#ifdef GAME_DLL
			if ( ( psurf != NULL ) && ( psurf->game.material == CHAR_TEX_GLASS ) && ( tr.m_pEnt->ClassMatches( "func_breakable" ) ) )
			{
				bHitGlass = true;
			}
#endif
		}

		if ( ( info.m_iTracerFreq != 0 ) && ( tracerCount++ % info.m_iTracerFreq ) == 0 && ( bHitGlass == false ) )
		{
			if ( bDoServerEffects == true )
			{
				Vector vecTracerSrc = vec3_origin;
				ComputeTracerStartPosition( info.m_vecSrc, &vecTracerSrc );

				trace_t Tracer;
				Tracer = tr;
				Tracer.endpos = vecTracerDest;

				MakeTracer( vecTracerSrc, Tracer, pAmmoDef->TracerType(info.m_iAmmoType) );
			}
			else
			{
				bDoTracers = true;
			}
		}

		//NOTENOTE: We could expand this to a more general solution for various material penetration types (wood, thin metal, etc)
		if( psurf != NULL && !(tr.m_pEnt && tr.m_pEnt->IsPlayer()) ) // hit something other than glass
		{
			HandleBulletPenetration( info, psurf->game.material, tr, vecFinalDir, &traceFilter );	
		}
		else if( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			HandleBulletPenetration( info, CHAR_TEX_FLESH, tr, vecFinalDir, &traceFilter );
		}
		else
		{
			DevMsg( 1, "Warning: No surface\n" );
			HandleBulletPenetration( info, CHAR_TEX_WOOD, tr, vecFinalDir, &traceFilter );	
		}

#ifdef GAME_DLL
		// See if we should pass through glass
		/*if ( bHitGlass )
		{
			HandleShotImpactingGlass( info, tr, vecFinalDir, &traceFilter );
		}*/
#endif

		iSeed++;
	}

#if defined( HL2MP ) && defined( GAME_DLL )
	if ( bDoServerEffects == false )
	{
		TE_HL2MPFireBullets( entindex(), tr.startpos, info.m_vecDirShooting, info.m_iAmmoType, iEffectSeed, info.m_iShots, info.m_vecSpread.x, bDoTracers, bDoImpacts );
	}
#endif

#ifdef GAME_DLL
	ApplyMultiDamage();
#endif

	if ( sv_showimpacts.GetBool() )
	{
		float flFade = sv_impactsfadetime.GetFloat();

#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawClientHitboxes( flFade, true );
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawServerHitboxes( flFade, true );
		}
#endif
	}

#ifdef GAME_DLL
	// Move other players back to history positions based on local player's lag
	if( doLagComp )
		lagcompensation->FinishLagCompensation( this );
#endif

}

/*
#define	CHAR_TEX_CONCRETE	'C'
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_DIRT		'D'
#define CHAR_TEX_VENT		'V'
#define CHAR_TEX_GRATE		'G'
#define CHAR_TEX_TILE		'T'
#define CHAR_TEX_SLOSH		'S'
#define CHAR_TEX_WOOD		'W'
#define CHAR_TEX_COMPUTER	'P'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_FLESH		'F'
#define CHAR_TEX_BLOODYFLESH	'B'
#define CHAR_TEX_CLIP		'I'
#define CHAR_TEX_ANTLION	'A'
#define CHAR_TEX_ALIENFLESH	'H'
#define CHAR_TEX_FOLIAGE	'O'
#define CHAR_TEX_SAND		'N'
#define CHAR_TEX_PLASTIC	'L'
*/

float CHajPlayer::GetMaxPenetrationDistance( unsigned short surfaceType )
{
	CHAJWeaponBase *pWeapon = dynamic_cast< CHAJWeaponBase* >( GetActiveWeapon() );
	float fPenetration = 12.0f;

	if( pWeapon )
	{
		fPenetration = pWeapon->GetMaxPenetrationDistance( surfaceType );
	}
	else
	{
		DevMsg( 1, "Weapon was NULL\n" );

		switch( surfaceType )
		{
			case CHAR_TEX_GLASS:
				fPenetration = 64.0f;
				break;

			case CHAR_TEX_CONCRETE:
				fPenetration = 12.0f; //12 inches of concrete max
				break;
			case CHAR_TEX_METAL:
				fPenetration = 6.0f;
				break;
			case CHAR_TEX_SAND:
				fPenetration = 3.0f;
				break;
			case CHAR_TEX_VENT:
				fPenetration = 8.0f;
				break;
			case CHAR_TEX_WOOD | CHAR_TEX_TILE | CHAR_TEX_DIRT:
				fPenetration = 24.0f; 
				break;
			case CHAR_TEX_PLASTIC:
				fPenetration = 32.0f;
				break;
			default:
				fPenetration = 6.0f;
		}
	}

	return fPenetration;
}

float CHajPlayer::CalculatePenetrationDamageLoss( unsigned short surfaceType, float distanceTravelled )
{
	float fMultiplier = 1.0f;

	switch( surfaceType )
	{
		case CHAR_TEX_GLASS:
			fMultiplier = 0.25f;
			break;

		case CHAR_TEX_CONCRETE:
			fMultiplier = 7.0;
			break;
		case CHAR_TEX_METAL:
			fMultiplier = 6.0f;
			break;
		case CHAR_TEX_SAND:
		case CHAR_TEX_DIRT:
			fMultiplier = 5.0f;
			break;
		case CHAR_TEX_VENT:
			fMultiplier = 5.5;
			break;
		case CHAR_TEX_WOOD:
		case CHAR_TEX_TILE:
			fMultiplier = 4.0;
			break;

		case CHAR_TEX_FLESH:
		case CHAR_TEX_PLASTIC:
			fMultiplier = 6.5;
			break;
		default:
			fMultiplier = 1.0f;
			break;
	}

	return fMultiplier * distanceTravelled;
}

//-----------------------------------------------------------------------------
// Handle bullet penetration
//-----------------------------------------------------------------------------
void CHajPlayer::HandleBulletPenetration( const FireBulletsInfo_t &info, unsigned short surfaceType,
	const trace_t &tr, const Vector &vecDir, ITraceFilter *pTraceFilter )
{
	float damageLossMulti = 1.0f;

	// player penetration
	if( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
	{
		if( info.m_iDamage <= 50 ) // don't penetrate players if damage is less than 50 (rifles and MGs will most likely penetrate, SMGs won't)
			return;

		bool bWillDie = ( ( tr.m_pEnt->GetHealth() - info.m_iDamage ) <= 0 );

		if( tr.hitgroup == HITGROUP_HEAD && bWillDie )
			damageLossMulti = 0.33f;
		else if( tr.hitgroup == HITGROUP_HEAD && !bWillDie )
			damageLossMulti = 0.5f;
		else if( tr.hitgroup == HITGROUP_LEFTARM || tr.hitgroup == HITGROUP_RIGHTARM || tr.hitgroup == HITGROUP_LEFTLEG || tr.hitgroup == HITGROUP_RIGHTLEG )
			damageLossMulti = 0.75f; // less damage loss on the legs and arms
		else if( !bWillDie )
			return; // don't go through anywhere else if we're not going to die

	}

	// Move through the glass until we're at the other side
	Vector	testPos = tr.endpos + ( vecDir * GetMaxPenetrationDistance( surfaceType ) );

	CEffectData	data;

	data.m_vNormal = tr.plane.normal;
	data.m_vOrigin = tr.endpos;

	DispatchEffect( "Impact", data );

	trace_t	penetrationTrace;

	// Re-trace as if the bullet had passed right through
	UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, pTraceFilter, &penetrationTrace );

	// See if we found the surface again
	if ( penetrationTrace.startsolid || tr.fraction == 0.0f || penetrationTrace.fraction == 1.0f )
		return;

	//FIXME: This is technically frustrating MultiDamage, but multiple shots hitting multiple targets in one call
	//		 would do exactly the same anyway...

	// Impact the other side (will look like an exit effect)
	DoImpactEffect( penetrationTrace, GetAmmoDef()->DamageType(info.m_iAmmoType) );

	data.m_vNormal = penetrationTrace.plane.normal;
	data.m_vOrigin = penetrationTrace.endpos;
	
	DispatchEffect( "Impact", data );

	Vector vecDist = penetrationTrace.endpos - tr.endpos;
	float distanceTravelled = abs( vecDist.Length() );

	float damageLoss = CalculatePenetrationDamageLoss( surfaceType, distanceTravelled ) * damageLossMulti;

	if( info.m_iDamage - (int)damageLoss > 0 )
	{
		// Refire the round, as if starting from behind the glass
		FireBulletsInfo_t behindGlassInfo;
		behindGlassInfo.m_iShots = 1;
		behindGlassInfo.m_vecSrc = penetrationTrace.endpos;
		behindGlassInfo.m_vecDirShooting = vecDir;
		behindGlassInfo.m_vecSpread = vec3_origin;
		behindGlassInfo.m_flDistance = info.m_flDistance*( 1.0f - tr.fraction );
		behindGlassInfo.m_iAmmoType = info.m_iAmmoType;
		behindGlassInfo.m_iTracerFreq = info.m_iTracerFreq;
		behindGlassInfo.m_iDamage = info.m_iDamage - (int)damageLoss;
		behindGlassInfo.m_pAttacker = info.m_pAttacker ? info.m_pAttacker : this;
		behindGlassInfo.m_nFlags = info.m_nFlags;

		FireBullets( behindGlassInfo, false );

	}
}