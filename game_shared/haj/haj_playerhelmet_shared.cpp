//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  Shared HaJ client/server side helmet entity stuff.
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_playerhelmet_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Global array of helmet names. 
//
// This is shared client/server and also used by the gib code that spawns helmets
// when they fall off. Make sure they match the enums in haj_playerhelmet_shared.h

const char *g_pszHelmetModels[HELMET_COUNT] = 
{
	"models/player/helmets/test_helmet.mdl",
	"models/player/helmets/helmet_mkii.mdl",
	"models/player/helmets/helmet_mkii_net.mdl",
	"models/player/helmets/cap_comforter.mdl",
	"models/player/helmets/helmet_m40.mdl",
	"models/player/helmets/helmet_m40_net.mdl",
	"models/player/helmets/helmet_m40_camo.mdl",
	"models/player/helmets/cap_m43.mdl"
};