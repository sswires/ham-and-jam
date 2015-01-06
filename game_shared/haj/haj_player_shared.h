//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HAJ_PLAYER_SHARED_H
#define HAJ_PLAYER_SHARED_H
#pragma once
#include "studio.h"

#if defined( CLIENT_DLL )
#include "haj_player_c.h"
#endif

#if defined( CLIENT_DLL )
#define CHajPlayer C_HajPlayer
#endif


// HAJ PLAYER SPEEDS
#define	HAJ_WALK_SPEED 110
#define	HAJ_NORM_SPEED 170
#define	HAJ_SPRINT_SPEED 250

#endif