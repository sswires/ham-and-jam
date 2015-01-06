#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

#include "haj_player.h"
#include "haj_weapon_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar haj_item_staytime;

bool ITEM_GiveAmmoHAJ( CBasePlayer *pPlayer, int iMagazines, CHAJWeaponBase* pWeapon )
{
	if( pWeapon )
	{
		pPlayer->GiveAmmo( 1, pWeapon->m_iPrimaryAmmoType );
	}

	DevMsg( "pWeapon did not return anything\n" );
	return false;
}

class CBaseMGAmmo : public CItem
{
public:
	DECLARE_CLASS( CBaseMGAmmo, CItem );
	DECLARE_DATADESC();

	virtual void ItemThink( )
	{
		float dieTime = m_flSpawnTime + haj_item_staytime.GetFloat();

		if( gpGlobals->curtime >= dieTime )
		{
			UTIL_Remove( this );
		}
		
		SetNextThink( gpGlobals->curtime + 0.25 );
	}

	void Spawn( void )
	{ 
		Precache( );
		SetModel( GetCrateModel() );

		BaseClass::Spawn( );

		m_flSpawnTime = gpGlobals->curtime;
		SetThink( &CBaseMGAmmo::ItemThink );
	}

	/*virtual const char *GetWeaponClass( )
	{
		return "";
	}*/

	virtual const char *GetCrateModel( ) { return "models/surgeon/ammocan2.mdl"; }

	bool MyTouch( CBasePlayer *pPlayer )
	{	
		CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );
		CHajGameRules *pGamerules = HajGameRules();

		if( pHajPlayer && pHajPlayer->GetTeamNumber() == GetTeamNumber() && pHajPlayer->GetCurrentClass() == CLASS_SUPPORT )
		{
			CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase*>( pHajPlayer->Weapon_GetSlot( 1 ) );

			if( pWeapon )
			{
				DevMsg( 1, "Giving %s some %s ammo\n", pPlayer->GetPlayerName(), pWeapon->GetClassName() );

				if ( ITEM_GiveAmmoHAJ( pPlayer, 1, pWeapon ) )
				{
					CHajPlayer *pOwnerPlayer = ToHajPlayer( GetOwnerEntity() );
					
					if( pOwnerPlayer )
					{
						if( pGamerules && !pGamerules->IsFreeplay() )
						{
							pOwnerPlayer->m_iAmmoGiven += 1;
							int ceGiven = 0;

							if( pOwnerPlayer->m_iAmmoGiven % 2 )
							{
								pOwnerPlayer->IncrementFragCount( 1 );
								ceGiven = 1;
							}

							pOwnerPlayer->SendNotification( "#HaJ_GaveAmmo", NOTIFICATION_MENTIONS_PLAYER, ceGiven, pHajPlayer, pHajPlayer->GetPlayerName() );
							pHajPlayer->SendNotification( "#Haj_ReceivedAmmo", NOTIFICATION_MENTIONS_PLAYER, 0, pOwnerPlayer, pOwnerPlayer->GetPlayerName() );
						}
					}

					UTIL_Remove(this);
					return true;
				}
			}
		}

		return false;
	}

	void Precache( void )
	{
		PrecacheModel( GetCrateModel() );
	}
private:
	float m_flSpawnTime;
};

BEGIN_DATADESC( CBaseMGAmmo )
	DEFINE_THINKFUNC( ItemThink ),
END_DATADESC()

class CItem_AlliedMGMagazine_Bren1 : public CBaseMGAmmo
{
public:
	DECLARE_CLASS( CItem_AlliedMGMagazine_Bren1, CBaseMGAmmo );

	//const char *GetWeaponClass( ) { return "weapon_bren"; }
	const char *GetCrateModel( ) { return "models/surgeon/ammocan2.mdl"; }
};
LINK_ENTITY_TO_CLASS( item_haj_alliedmgmagazine, CItem_AlliedMGMagazine_Bren1 );


class CItem_AxisMGMagazine : public CBaseMGAmmo
{
public:
	DECLARE_CLASS( CItem_AxisMGMagazine, CBaseMGAmmo );


	//const char *GetWeaponClass( ) { return "weapon_mg34"; }
	const char *GetCrateModel( ) { return "models/surgeon/ammocan2.mdl"; }

};
LINK_ENTITY_TO_CLASS( item_haj_axismgmagazine, CItem_AxisMGMagazine );