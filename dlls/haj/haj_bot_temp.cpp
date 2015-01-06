//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic BOT handling.
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "in_buttons.h"
#include "movehelper_server.h"
#include "gameinterface.h"
#include "hl2mp_player.h"
#include "haj_player.h"
#include "haj_weapon_base.h"
#include "haj_misc.h"
#include "haj_gamerules.h"
#include "team.h" 

#include "nav_mesh.h"

class CSDKBot;
void Bot_Think( CSDKBot *pBot );

ConVar bot_forcefireweapon( "bot_forcefireweapon", "", 0, "Force bots with the specified weapon to fire." );
ConVar bot_forceattack2( "bot_forceattack2", "0", 0, "When firing, use attack2." );
ConVar bot_forceattackon( "bot_forceattackon", "0", 0, "When firing, don't tap fire, hold it down." );
ConVar bot_flipout( "bot_flipout", "0", 0, "When on, all bots fire their guns." );
ConVar bot_changeclass( "bot_changeclass", "1", 0, "Force all bots to change to the specified class." );
static ConVar bot_mimic( "bot_mimic", "0", 0, "Bot uses usercmd of player by index." );
static ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "0", 0, "Offsets the bot yaw." );

ConVar bot_sendcmd( "bot_sendcmd", "", 0, "Forces bots to send the specified command." );

ConVar bot_crouch( "bot_crouch", "0", 0, "Bot crouches" );

ConVar bot_speak( "bot_speak", "", 0, "Forces bots to yell stuff" );
ConVar bot_intelligence( "bot_intelligence", "0", FCVAR_GAMEDLL, "Yeah" );

static int g_CurBotNumber = 1;


// This is our bot class.
class CSDKBot : public CHajPlayer
{
public:

	virtual void	BotThink( CUserCmd &cmd );

	// bot states
	void			BotPatrolling( CUserCmd &cmd );
	void			BotRushingObjective( CUserCmd &cmd );
	void			BotCapturingObjective( CUserCmd &cmd );

	void			Navigate( CUserCmd &cmd );

	// killin'
	int				GetTargetWeight( CHajPlayer *pOther );
	bool			ValidateTarget( CHajPlayer *pOther );
	void			SelectCurrentTarget( void );

	void			AimAt( Vector &pos, CUserCmd &cmd );

	enum botState_e
	{
		BOT_STATE_WAITING,
		BOT_STATE_PATROL,
		BOT_STATE_RUSHING_OBJECTIVE,
		BOT_STATE_CAPTURING_OBJECTIVE
	};

	CHandle<CHajObjective> m_hObjectiveTarget;
	CHandle<CHajPlayer> m_hKillTarget;

	CNavArea *m_pCurrentNavTarget;

	bool			m_bEngagingTargets;

	int				m_iState;

	bool			m_bBackwards;

	float			m_flNextTurnTime;
	bool			m_bLastTurnToRight;

	float			m_flNextStrafeTime;
	float			m_flSideMove;

	QAngle			m_ForwardAngle;
	QAngle			m_LastAngles;
};

void CSDKBot::BotThink( CUserCmd &cmd )
{
	CHajGameRules *pRules = HajGameRules();

	if( pRules )
	{
		if( IsAlive() )
		{
			switch( m_iState )
			{
				case BOT_STATE_PATROL:
					BotPatrolling( cmd );
					break;

				case BOT_STATE_RUSHING_OBJECTIVE:
					BotRushingObjective( cmd );
					break;

				case BOT_STATE_CAPTURING_OBJECTIVE:
					BotCapturingObjective( cmd );
					break;

				case BOT_STATE_WAITING:
				default:
					
					m_iState = BOT_STATE_PATROL;
			}

			Navigate( cmd );
		}
		else
		{
			m_iState = BOT_STATE_WAITING;

			if( pRules->FPlayerCanRespawn( this ) )
			{
				int iNewClass = random->RandomInt( CLASS_RIFLEMAN, CLASS_SUPPORT );

				if( !IsClassFull( iNewClass ) )
				{
					SetDesiredClass( iNewClass );
				}

				Spawn();
			}
		}
	}
}

void CSDKBot::BotRushingObjective( CUserCmd &cmd )
{
	// we need a target
	if( m_hObjectiveTarget.Get() == NULL )
	{
		const CUtlLinkedList<CHajObjective*>& objectives = _objectiveman.GetObjectives();	

		// find appropriate objective to rush
		for( int i = 0; i < objectives.Count(); i++ )
		{
			if( !objectives[i] )
				continue;

			if( objectives[i]->CanBeCapturedByTeam( GetTeamNumber() ) )
				m_hObjectiveTarget.Set( objectives[i] );
		}
	}

	// we failed
	if( m_hObjectiveTarget.Get() == NULL )
	{
		m_iState = BOT_STATE_PATROL;
	}
	else
	{
		// navigate to objective
		CNavArea *pObjectiveArea = TheNavMesh->GetNearestNavArea( m_hObjectiveTarget.Get()->GetAbsOrigin(), true );

		if( !pObjectiveArea )
		{
			m_iState = BOT_STATE_PATROL;
			m_hObjectiveTarget.Set( NULL );
		}
		else // we know where the objective is
		{
			Vector playerPos = GetAbsOrigin();
			NavDirType direction = pObjectiveArea->ComputeDirection( &playerPos );
		}

		// sprint if we have enough suit power
		if( SuitPower_GetCurrentPercentage() > 0 )
			cmd.buttons |= IN_SPEED;

		cmd.forwardmove = MaxSpeed();
	}

}

void CSDKBot::BotCapturingObjective( CUserCmd &cmd )
{
	// completed objective or it doesn't exist anymore
	if( m_hObjectiveTarget.Get() == NULL || !m_hObjectiveTarget->CanBeCapturedByTeam(GetTeamNumber()) )
	{
		m_hObjectiveTarget.Set( NULL );
		m_iState = BOT_STATE_PATROL;
	}
}

void CSDKBot::BotPatrolling( CUserCmd &cmd )
{
	if( m_hKillTarget.Get() == NULL )
	{
		SelectCurrentTarget();
	}

	CHajPlayer *pTarget = m_hKillTarget.Get();
	m_bEngagingTargets = false;

	if( pTarget && ValidateTarget( pTarget ) )
	{
		// aim
		AimAt( pTarget->EyePosition(), cmd );

		CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase*>( GetActiveWeapon() );

		if( pWeapon )
		{
			if( pWeapon->Clip1() / pWeapon->GetMaxClip1() < 0.2 )
			{
				cmd.buttons |= IN_RELOAD;
			}
			else
			{
				Vector aimDir = ToBasePlayer(this)->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
				trace_t tr;

				UTIL_TraceLine( EyePosition(), aimDir * 1024, MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr );

				if( tr.DidHitNonWorldEntity() && tr.m_pEnt == pTarget )
				{
					if( pWeapon && pWeapon->m_flNextPrimaryAttack >= gpGlobals->curtime )
					{
						cmd.buttons |= IN_ATTACK;
					}

					m_bEngagingTargets = true;
				}

			}
		}
	}
}

int CSDKBot::GetTargetWeight( CHajPlayer *pOther )
{
	trace_t tr;
	UTIL_TraceLine( EyePosition(), pOther->EyePosition(), MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr );

	if( tr.DidHitNonWorldEntity() && tr.m_pEnt == pOther )
	{
		// calculate score
		return pOther->FragCount() + 100;
	}

	return 0;
}

bool CSDKBot::ValidateTarget( CHajPlayer *pOther )
{
	return (GetTargetWeight(pOther) > 0);
}

void CSDKBot::SelectCurrentTarget( void )
{
	CUtlMap<int,CHajPlayer*> m_PlayersWeCanKill;
	m_PlayersWeCanKill.SetLessFunc(IntLessFunc_Descending);

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CHajPlayer *pPlayer = ToHajPlayer( UTIL_PlayerByIndex(i) );

		if( !pPlayer || pPlayer->GetTeamNumber() == GetTeamNumber() || pPlayer->IsObserver() || !pPlayer->IsAlive() )
			continue;

		m_PlayersWeCanKill.Insert( GetTargetWeight(pPlayer), pPlayer );
	}

	m_hKillTarget.Set( m_PlayersWeCanKill.Element( m_PlayersWeCanKill.FirstInorder() ) );
}

// this will interpolate the view angle to the chosen position
void CSDKBot::AimAt( Vector &pos, CUserCmd &cmd  )
{
	Vector direction = ( pos - EyePosition() );
	VectorNormalize( direction );

	QAngle vangle;
	VectorAngles( direction, vangle );

	float p = ApproachAngle( direction[PITCH], m_LastAngles[PITCH], 5.0f * gpGlobals->frametime );
	float y = ApproachAngle( direction[YAW], m_LastAngles[YAW], 15.0f * gpGlobals->frametime );

	direction[PITCH] = p;
	direction[YAW] = y;

	cmd.viewangles = vangle;
}

void CSDKBot::Navigate( CUserCmd &cmd )
{
	CNavArea *pCurrentNav = TheNavMesh->GetNavArea( GetAbsOrigin() );

	// some basic movement
	if( CanVault() )
		cmd.buttons |= IN_JUMP;

	// navigate randomly
	if( pCurrentNav )
	{
		// finished navigating to target
		if( m_pCurrentNavTarget == pCurrentNav )
		{
			Vector vCenter = pCurrentNav->GetCenter();
			AimAt( vCenter, cmd );

			m_pCurrentNavTarget = NULL;
		}

		// find direction to go in
		if( !m_pCurrentNavTarget )
		{
			NavDirType direction = SOUTH;

			if( m_hKillTarget.Get() != NULL )
			{
				Vector vInterestPos = m_hKillTarget.Get()->GetAbsOrigin();
				direction = pCurrentNav->ComputeDirection( &vInterestPos );
			}

			if( pCurrentNav->GetAdjacentCount( direction ) )
				m_pCurrentNavTarget = pCurrentNav->GetAdjacentArea( direction, 0 );
			else
			{
				const CNavArea::ApproachInfo *pApproach = pCurrentNav->GetApproachInfo( 0 );
				m_pCurrentNavTarget = pApproach->next.area;
			}

			if( !m_pCurrentNavTarget )
			{
				m_pCurrentNavTarget = TheNavMesh->GetNearestNavArea( GetAbsOrigin(), true );
			}
		}

		if( m_pCurrentNavTarget )
		{
			Vector approachPos;
			m_pCurrentNavTarget->GetClosestPointOnArea( GetAbsOrigin(), &approachPos );

			if( !m_bEngagingTargets )
			{
				AimAt( approachPos, cmd );
				cmd.viewangles[PITCH] = 0;
				cmd.forwardmove = MaxSpeed();

				if( SuitPower_GetCurrentPercentage() > 25 )
					cmd.buttons |= IN_SPEED;
			}
		}
	}

}

const char* FindEngineArg( const char *pName )
{
   int nArgs = engine->Cmd_Argc();
   for ( int i=1; i < nArgs; i++ )
   {
      if ( stricmp( engine->Cmd_Argv(i), pName ) == 0 )
         return (i+1) < nArgs ? engine->Cmd_Argv(i+1) : "";
   }
   return 0;
}

int FindEngineArgInt( const char *pName, int defaultVal )
{
   const char *pVal = FindEngineArg( pName );
   if ( pVal )
      return atoi( pVal );
   else
      return defaultVal;
}


LINK_ENTITY_TO_CLASS( sdk_bot, CSDKBot );

class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CSDKBot *pPlayer = static_cast<CSDKBot *>( CreateEntityByName( "sdk_bot" ) );
		if ( pPlayer )
		{
			pPlayer->SetPlayerName( playername );
		}

		return pPlayer;
	}
};

#define BOT_NAME_COUNT 4
const char* g_BotNames[] = {
	"Pvt Pudding",
	"0wn3r",
	"Jed",
	"Ginger Lord",
};

//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool bFrozen )
{
	char botname[ 64 ];

	if( g_CurBotNumber <= BOT_NAME_COUNT )
		Q_snprintf( botname, sizeof( botname ), "[HaJB]%s", g_BotNames[g_CurBotNumber-1] );
	else
		Q_snprintf( botname, sizeof( botname ), "[HaJB]%s(%d)", g_BotNames[random->RandomInt(0,BOT_NAME_COUNT-1)], g_CurBotNumber );

	// This trick lets us create a CSDKBot for this client instead of the CSDKPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( botname );
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Msg( "Failed to create Bot.\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CSDKBot *pPlayer = ((CSDKBot*)CBaseEntity::Instance( pEdict ));

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	pPlayer->HandleCommand_JoinTeam( TEAM_AUTO );
	pPlayer->SetDesiredClass( bot_changeclass.GetInt() );
	pPlayer->Spawn();
	pPlayer->m_iState = CSDKBot::BOT_STATE_WAITING;

	g_CurBotNumber++;

	return pPlayer;
}

// Handler for the "bot" command.
void BotAdd_f()
{
	extern int FindEngineArgInt( const char *pName, int defaultVal );
	extern const char* FindEngineArg( const char *pName );

	// Look at -count.
	int count = FindEngineArgInt( "-count", 1 );
	count = clamp( count, 1, 16 );

	// Look at -frozen.
	bool bFrozen = !!FindEngineArg( "-frozen" );
		
	// Ok, spawn all the bots.
	while ( --count >= 0 )
	{
		BotPutInServer( bFrozen );
	}
}
ConCommand cc_Bot( "bot_add", BotAdd_f, "Add a bot.", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHajPlayer *pPlayer = ToHajPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CSDKBot *pBot = dynamic_cast< CSDKBot* >( pPlayer );
			if ( pBot )
				Bot_Think( pBot );
		}
	}
}

bool Bot_RunMimicCommand( CUserCmd& cmd )
{
	if ( bot_mimic.GetInt() <= 0 )
		return false;

	if ( bot_mimic.GetInt() > gpGlobals->maxClients )
		return false;

	
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt()  );
	if ( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand() )
		return false;

	cmd = *pPlayer->GetLastUserCommand();
	cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

	if( bot_crouch.GetInt() )
		cmd.buttons |= IN_DUCK;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input  : *fakeclient - 
//			*viewangles - 
//			forwardmove - 
//			m_flSideMove - 
//			upmove - 
//			buttons - 
//			impulse - 
//			msec - 
// Output : 	virtual void
//-----------------------------------------------------------------------------
static void RunPlayerMove( CHajPlayer *fakeclient, CUserCmd &cmd, float frametime )
{
	if ( !fakeclient )
		return;

	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	fakeclient->SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( fakeclient );
	fakeclient->PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	fakeclient->SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	fakeclient->pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}



void Bot_UpdateStrafing( CSDKBot *pBot, CUserCmd &cmd )
{
	if ( gpGlobals->curtime >= pBot->m_flNextStrafeTime )
	{
		pBot->m_flNextStrafeTime = gpGlobals->curtime + 1.0f;

		if ( random->RandomInt( 0, 5 ) == 0 )
		{
			pBot->m_flSideMove = -600.0f + 1200.0f * random->RandomFloat( 0, 2 );
		}
		else
		{
			pBot->m_flSideMove = 0;
		}
		cmd.sidemove = pBot->m_flSideMove;

		if ( random->RandomInt( 0, 20 ) == 0 )
		{
			pBot->m_bBackwards = true;
		}
		else
		{
			pBot->m_bBackwards = false;
		}
	}
}


void Bot_UpdateDirection( CSDKBot *pBot )
{
	float angledelta = 15.0;
	QAngle angle;

	int maxtries = (int)360.0/angledelta;

	if ( pBot->m_bLastTurnToRight )
	{
		angledelta = -angledelta;
	}

	angle = pBot->GetLocalAngles();

	trace_t trace;
	Vector vecSrc, vecEnd, forward;
	while ( --maxtries >= 0 )
	{
		AngleVectors( angle, &forward );

		vecSrc = pBot->GetLocalOrigin() + Vector( 0, 0, 36 );

		vecEnd = vecSrc + forward * 10;

		UTIL_TraceHull( vecSrc, vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, 
			MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &trace );

		if ( trace.fraction == 1.0 )
		{
			if ( gpGlobals->curtime < pBot->m_flNextTurnTime )
			{
				break;
			}
		}

		angle.y += angledelta;

		if ( angle.y > 180 )
			angle.y -= 360;
		else if ( angle.y < -180 )
			angle.y += 360;

		pBot->m_flNextTurnTime = gpGlobals->curtime + 2.0;
		pBot->m_bLastTurnToRight = random->RandomInt( 0, 1 ) == 0 ? true : false;

		pBot->m_ForwardAngle = angle;
		pBot->m_LastAngles = angle;
	}
	
	pBot->SetLocalAngles( angle );
}


void Bot_FlipOut( CSDKBot *pBot, CUserCmd &cmd )
{
	if ( bot_flipout.GetInt() > 0 && pBot->IsAlive() )
	{
		if ( bot_forceattackon.GetBool() || (RandomFloat(0.0,1.0) > 0.5) )
		{
			cmd.buttons |= bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
		}

		if ( bot_flipout.GetInt() >= 2 )
		{
			QAngle angOffset = RandomAngle( -1, 1 );

			pBot->m_LastAngles += angOffset;

			for ( int i = 0 ; i < 2; i++ )
			{
				if ( fabs( pBot->m_LastAngles[ i ] - pBot->m_ForwardAngle[ i ] ) > 15.0f )
				{
					if ( pBot->m_LastAngles[ i ] > pBot->m_ForwardAngle[ i ] )
					{
						pBot->m_LastAngles[ i ] = pBot->m_ForwardAngle[ i ] + 15;
					}
					else
					{
						pBot->m_LastAngles[ i ] = pBot->m_ForwardAngle[ i ] - 15;
					}
				}
			}

			pBot->m_LastAngles[ 2 ] = 0;

			pBot->SetLocalAngles( pBot->m_LastAngles );
		}
	}
}


void Bot_HandleSendCmd( CSDKBot *pBot )
{
	if ( strlen( bot_sendcmd.GetString() ) > 0 )
	{
		//send the cmd from this bot
		pBot->ClientCommand( bot_sendcmd.GetString() );

		bot_sendcmd.SetValue("");
	}
}

void Bot_HandleSpeak( CSDKBot *pBot )
{
	if ( strlen( bot_speak.GetString() ) > 0 )
	{
		//send the cmd from this bot
		pBot->DoVoiceCommand( bot_speak.GetString() );

		bot_speak.SetValue("");
	}
}


// If bots are being forced to fire a weapon, see if I have it
void Bot_ForceFireWeapon( CSDKBot *pBot, CUserCmd &cmd )
{
	if ( bot_forcefireweapon.GetString() )
	{
		CBaseCombatWeapon *pWeapon = pBot->Weapon_OwnsThisType( bot_forcefireweapon.GetString() );
		if ( pWeapon )
		{
			// Switch to it if we don't have it out
			CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();

			// Switch?
			if ( pActiveWeapon != pWeapon )
			{
				pBot->Weapon_Switch( pWeapon );
			}
			else
			{
				// Start firing
				// Some weapons require releases, so randomise firing
				if ( bot_forceattackon.GetBool() || (RandomFloat(0.0,1.0) > 0.5) )
				{
					cmd.buttons |= bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
				}
			}
		}
	}
}


void Bot_SetForwardMovement( CSDKBot *pBot, CUserCmd &cmd )
{
	if ( !pBot->IsEFlagSet(EFL_BOT_FROZEN) )
	{
		if ( pBot->m_iHealth == 100 )
		{
			cmd.forwardmove = 600 * ( pBot->m_bBackwards ? -1 : 1 );
			if ( pBot->m_flSideMove != 0.0f )
			{
				cmd.forwardmove *= random->RandomFloat( 0.1, 1.0f );
			}
		}
		else
		{
			// Stop when shot
			cmd.forwardmove = 0;
		}
	}
}


void Bot_HandleRespawn( CSDKBot *pBot, CUserCmd &cmd )
{
	// Wait for Reinforcement wave
	if ( !pBot->IsAlive() )
	{
		// Try hitting my buttons occasionally
		pBot->SetDesiredClass( RandomInt( CLASS_UNASSIGNED+1, CLASS_LAST-1 ) );
		pBot->Spawn();
	}
}


//-----------------------------------------------------------------------------
// Run this Bot's AI for one frame.
//-----------------------------------------------------------------------------
void Bot_Think( CSDKBot *pBot )
{
	// Make sure we stay being a bot
	pBot->AddFlag( FL_FAKECLIENT );


	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );
	
	
	// Finally, override all this stuff if the bot is being forced to mimic a player.
	if( bot_intelligence.GetBool() )
	{
		pBot->BotThink( cmd );
	}
	else
	{
		if ( !Bot_RunMimicCommand( cmd ) || !pBot->IsAlive() )
		{
			cmd.sidemove = pBot->m_flSideMove;

			if ( pBot->IsAlive() && (pBot->GetSolid() == SOLID_BBOX) )
			{
				Bot_SetForwardMovement( pBot, cmd );

				// Only turn if I haven't been hurt
				if ( !pBot->IsEFlagSet(EFL_BOT_FROZEN) && pBot->m_iHealth == 100 )
				{
					Bot_UpdateDirection( pBot );
					Bot_UpdateStrafing( pBot, cmd );
				}

				// Handle console settings.
				Bot_ForceFireWeapon( pBot, cmd );
				Bot_HandleSendCmd( pBot );
				Bot_HandleSpeak( pBot );
			}
			else
			{
				Bot_HandleRespawn( pBot, cmd );
			}

			Bot_FlipOut( pBot, cmd );
			
			cmd.viewangles = pBot->GetLocalAngles();
			cmd.upmove = 0;
			cmd.impulse = 0;
		}
	}

	// Fix up the m_fEffects flags
	pBot->PostClientMessagesSent();

	float frametime = gpGlobals->frametime;
	RunPlayerMove( pBot, cmd, frametime );
}


