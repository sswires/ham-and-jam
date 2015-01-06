//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef IN_BUTTONS_H
#define IN_BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_PRONE		(1 << 3)	// *HAJ 020* - Jed
#define IN_FORWARD		(1 << 4)
#define IN_BACK			(1 << 5)
#define IN_USE			(1 << 6)
#define IN_CANCEL		(1 << 7)
#define IN_LEFT			(1 << 8)
#define IN_RIGHT		(1 << 9)
#define IN_MOVELEFT		(1 << 10)
#define IN_MOVERIGHT	(1 << 11)
#define IN_ATTACK2		(1 << 12)
#define IN_RUN			(1 << 13)
#define IN_RELOAD		(1 << 14)
#define IN_ALT1			(1 << 15)
#define IN_ALT2			(1 << 16)
#define IN_SCORE		(1 << 17)   // Used by client.dll for when scoreboard is held down
#define IN_SPEED		(1 << 18)	// Player is holding the speed key
#define IN_WALK			(1 << 19)	// Player holding walk key
#define IN_ZOOM			(1 << 20)	// Zoom key for HUD zoom
#define IN_WEAPON1		(1 << 21)	// weapon defines these bits
#define IN_WEAPON2		(1 << 22)	// weapon defines these bits
#define IN_BULLRUSH		(1 << 23)
#define IN_GRENADE1		(1 << 24)	// grenade 1
#define IN_GRENADE2		(1 << 25)	// grenade 2
#define IN_SHOWOBJLOC	(1 << 26)	// show objective locations

#endif // IN_BUTTONS_H