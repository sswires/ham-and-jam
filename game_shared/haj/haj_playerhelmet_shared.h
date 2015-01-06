//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  Shared HaJ client/server side helmet entity stuff.
//
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_HELMET_SHARED_H
#define HAJ_HELMET_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// Helmet enumerations. Make these match up with the global helmet model array names.
//
enum
{
	HELMET_NONE = 0,

// commonwealth headgear
	HELMET_MK2_STANDARD,
	HELMET_MK2_NET,
	CAP_COMFORTER,

// german headgear
	HELMET_M40_STANDARD,
	HELMET_M40_NET,
	HELMET_M40_CAMO,
	CAP_M43_STANDARD,

	HELMET_COUNT,
};

#endif // HAJ_HELMET_SHARED_H