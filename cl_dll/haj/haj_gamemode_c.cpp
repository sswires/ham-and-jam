#include "cbase.h"
#include "haj_gamemode_c.h"
#include "haj_player_c.h"
#include "haj_bombzone_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( HajGameMode, DT_HajGameMode )

BEGIN_NETWORK_TABLE( CHajGameMode, DT_HajGameMode )
END_NETWORK_TABLE()


bool CHajGameMode::CanPlantBomb( CBasePlayer* pPlanter, C_BombZone *pZone )
{
	C_HajPlayer *pPlayer = ToHajPlayer( pPlanter );

	if( pPlayer->IsInBombZone() && pZone && pZone->CanBeCapturedByTeam( pPlayer->GetTeamNumber() ) )
		return true;

	return false;
}
