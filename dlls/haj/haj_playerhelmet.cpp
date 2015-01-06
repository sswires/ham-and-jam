//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  HaJ server side helmet entity.
// Notes:    Simple helmet model attached to the players head which falls off sometimes. :)
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_playerhelmet.h"
#include "haj_playerhelmet_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( CPlayerHelmet, DT_PlayerHelmet )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( player_helmet, CPlayerHelmet );

BEGIN_DATADESC( CPlayerHelmet )
END_DATADESC()

// --------------------------------
// Constructor
// --------------------------------
CPlayerHelmet::CPlayerHelmet( void )
{
}

// --------------------------------
// Destructor
// --------------------------------
CPlayerHelmet::~CPlayerHelmet( void )
{
}

// --------------------------------
// Precache
// --------------------------------
void CPlayerHelmet::Precache( void )
{
}


// --------------------------------
// Spawn
// --------------------------------
void CPlayerHelmet::Spawn( void )
{
	// precache all the models
	Precache();

	// just spawn for now
	BaseClass::Spawn();
}