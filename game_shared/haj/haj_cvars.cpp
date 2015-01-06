// *HaJ 202* - SteveUK
// Use this file for adding new serverside variables, for easy access.

#include "cbase.h"
#ifdef GAME_DLL
	#include "hl2mp_cvars.h"
#endif
#include "haj_cvars.h"

#define HAJ_CVAR_FLAGS FCVAR_REPLICATED|FCVAR_ARCHIVE|FCVAR_NOTIFY
#define HAJ_CVAR_FLAGS_NO_SAVE FCVAR_REPLICATED|FCVAR_NOTIFY

// How often a player can change what class they are
ConVar haj_classchangefreq( "haj_player_classchangefreq", "5", HAJ_CVAR_FLAGS, "How often a player may change their class in seconds" );
ConVar haj_allowthirdperson( "haj_allowthirdperson", "0", ( HAJ_CVAR_FLAGS_NO_SAVE | FCVAR_CHEAT ), "Allow thirdperson? =2 to force" );

ConVar haj_wave_time( "haj_wave_time", "10", HAJ_CVAR_FLAGS, "How often waves spawn" );
ConVar haj_item_staytime( "haj_item_staytime", "90", HAJ_CVAR_FLAGS, "How often waves spawn" );
ConVar haj_intermission_length( "haj_intermission_length", "30", HAJ_CVAR_FLAGS, "Intermission length");
ConVar haj_alltalk_during_interval( "haj_intermission_alltalk", "1", HAJ_CVAR_FLAGS, "Allow all players to talk to each other during free play and end game screen?" );
ConVar haj_nocollideteam( "haj_nocollideteam", "0", HAJ_CVAR_FLAGS, "No collide between teammates?" );

ConVar haj_support_grenades( "haj_support_grenades", "1", HAJ_CVAR_FLAGS_NO_SAVE, "Frag Grenade count for support class.", true, 0, true, 2 );
ConVar haj_assault_grenades( "haj_assault_grenades", "2", HAJ_CVAR_FLAGS_NO_SAVE, "Frag Grenade count for assault class.", true, 0, true, 2);
ConVar haj_rifleman_grenades( "haj_rifleman_grenades", "2", HAJ_CVAR_FLAGS_NO_SAVE, "Frag Grenade count for rifleman class.", true, 0, true, 2);

ConVar haj_allowunbalancedteams( "haj_allowunbalancedteams", "0", HAJ_CVAR_FLAGS, "Allow players to join 'full' teams?" );
ConVar haj_autoteambalance( "haj_autoteambalance", "1", HAJ_CVAR_FLAGS, "Auto-balance teams?");
ConVar haj_autoteambalance_sortdelay( "haj_autoteambalance_sortdelay", "15", HAJ_CVAR_FLAGS, "Delay in re-assigning teams" );
ConVar haj_switchteams( "haj_switchteams", "0", HAJ_CVAR_FLAGS, "Switch teams at end of round (=1)" );

ConVar haj_round_intermission( "haj_round_intermission", "7", HAJ_CVAR_FLAGS, "Number of seconds between round win and reset" );
ConVar haj_round_freezeplayers( "haj_round_freezeplayers", "0", HAJ_CVAR_FLAGS, "Freeze players between rounds" );

// PUSH/PULL
ConVar haj_pp_roundtime( "haj_pp_roundtime", "10", HAJ_CVAR_FLAGS, "Round time limit in push/pull gamemode" );
ConVar haj_pp_roundlimit( "haj_pp_roundlimit", "5", HAJ_CVAR_FLAGS, "Number of rounds per game in push/pull gamemode" );
ConVar haj_pp_scorelimit( "haj_pp_scorelimit", "0", HAJ_CVAR_FLAGS, "If >0, limit the number of rounds to win in push/pull" );

// ATTACK/DEFEND
ConVar haj_attackdefend_roundlimit( "haj_attackdefend_roundlimit", "3", HAJ_CVAR_FLAGS, "Number of rounds per game in attack/defend gamemode" );
ConVar haj_attackdefend_scorelimit( "haj_attackdefend_scorelimit", "2", HAJ_CVAR_FLAGS, "Score limit in attack/defend" );

// DESTROY
ConVar haj_destroy_roundlimit( "haj_destroy_roundlimit", "3", HAJ_CVAR_FLAGS, "Number of round per game in Destroy" );
ConVar haj_destroy_scorelimit( "haj_destroy_scorelimit", "0", HAJ_CVAR_FLAGS, "Score limit in Destroy" );

// SABOTAGE
ConVar haj_rbdestroy_roundtime( "haj_sab_roundtime", "3.5", HAJ_CVAR_FLAGS, "Duration of rounds in round-based destroy");
ConVar haj_rbdestroy_roundlimit( "haj_sab_roundlimit", "5", HAJ_CVAR_FLAGS, "Number of rounds per game in round-based destroy");
ConVar haj_rbdestroy_planttime( "haj_sab_planttime", "5", HAJ_CVAR_FLAGS, "Time it takes to plant the bomb in round-based destroy");
ConVar haj_rbdestroy_defusetime( "haj_sab_defusetime", "5", HAJ_CVAR_FLAGS, "Time it takes to defuse the bomb in round-based destroy");
ConVar haj_rbdestroy_bombtime( "haj_sab_bombtime", "35", HAJ_CVAR_FLAGS, "Time it takes to for bomb to explode in Sabotage");
ConVar haj_sab_scorelimit( "haj_sab_scorelimit", "0", HAJ_CVAR_FLAGS, "Score limit in round based destroy" );

ConVar haj_freeplaytime( "haj_freeplaytime", "60", HAJ_CVAR_FLAGS, "Duration of free play time at the begining of a map, gives everyone a chance to spawn before the real game starts." );
ConVar haj_freeplay_ff( "haj_freeplay_ff", "0", HAJ_CVAR_FLAGS, "Enable FF for free play only?" );
ConVar haj_freezetime( "haj_freezetime", "7", HAJ_CVAR_FLAGS, "Time spent frozen at the begining of the round" );

ConVar haj_player_allow_jump( "haj_player_allow_jump", "1", HAJ_CVAR_FLAGS, "Allow jumping? " );
ConVar haj_player_allow_vault( "haj_player_allow_vault", "0", ( HAJ_CVAR_FLAGS_NO_SAVE | FCVAR_CHEAT ), "Allow vaulting over walls" ); // disable vaulting for first release

ConVar haj_realism_smoke_choke( "haj_realism_smoke_choke", "0", HAJ_CVAR_FLAGS, "People choke on smoke grenades?" );
ConVar haj_realism_sprinting_backpedal( "haj_realism_sprinting_backpedal", "0", HAJ_CVAR_FLAGS, "When disabled, you can't walk backwards when you sprint" );

// Damage multiplers
//ConVar haj_dmg_head( "haj_dmg_head", "3", FCVAR_GAMEDLL | FCVAR_REPLICATED| FCVAR_ARCHIVE | FCVAR_SPONLY| FCVAR_CHEAT, "Damage multiplier for head" );
//ConVar haj_dmg_chest( "haj_dmg_chest", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_ARCHIVE | FCVAR_SPONLY |FCVAR_CHEAT, "Damage multiplier for chest" );
//ConVar haj_dmg_stomach( "haj_dmg_stomach", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_ARCHIVE | FCVAR_SPONLY | FCVAR_CHEAT, "Damage multiplier for stomach" );
//ConVar haj_dmg_arms( "haj_dmg_arms", "0.6", FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_ARCHIVE | FCVAR_SPONLY | FCVAR_CHEAT, "Damage multiplier for arms" );
//ConVar haj_dmg_legs( "haj_dmg_legs", "0.6", FCVAR_GAMEDLL | FCVAR_REPLICATED |FCVAR_ARCHIVE | FCVAR_SPONLY | FCVAR_CHEAT, "Damage multiplier for legs" );
//ConVar haj_dmg_fallout( "haj_dmg_fallout", "0.02", FCVAR_GAMEDLL | FCVAR_REPLICATED |FCVAR_ARCHIVE | FCVAR_SPONLY | FCVAR_CHEAT, "How much damage decreases per inch once the bullet hits past its maximum distance" );

ConVar haj_allow_grenade_bar( "haj_allow_grenade_bar", "0", HAJ_CVAR_FLAGS, "Allow players to use the grenade fuse timer bar" );

ConVar haj_bomb_test_mode( "haj_bomb_test_mode", "0", HAJ_CVAR_FLAGS_NO_SAVE, "Enable for bomb testing");