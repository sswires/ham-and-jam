//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  HaJ client side helmet entity.
// Notes:    Simple helmet model attached to the players head which falls off sometimes. :)
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "input.h"
#include "haj_playerhelmet_shared.h"
#include "haj_playerhelmet_c.h"
#include "haj_player_c.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_PlayerHelmet, DT_PlayerHelmet, CPlayerHelmet )
END_RECV_TABLE()

// Constructor
C_PlayerHelmet::C_PlayerHelmet( void )
{
}

// Deconstructor
C_PlayerHelmet::~C_PlayerHelmet( void )
{
}

bool C_PlayerHelmet::ShouldDraw( void )
{
	// TODO: This needs some refinement...

	// if we don't have an owner entity...
	if ( !GetOwnerEntity() )
		return BaseClass::ShouldDraw();
		
	if( GetOwnerEntity() == C_HajPlayer::GetLocalHajPlayer() )
		return input->CAM_IsThirdPerson();

	// decide if we should draw based on the owner.
	return GetOwnerEntity()->ShouldDraw();
/*

	// get a hook to the player
	C_HajPlayer *pPlayer  = (C_HajPlayer*) C_BaseEntity::Instance( GetOwnerEntity() );
	
	if ( pPlayer == NULL)
	{
		if ( GetOwnerEntity()->ShouldDraw() && pPlayer->GetHelmet() > 0 )
			return true;
		else
			return false;
	}
	else
		return false;
*/
}