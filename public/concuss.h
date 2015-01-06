//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Network data for concuss effect.
//
// $NoKeywords: $
//=======================================================================//

#ifndef CONCUSS_H
#define CONCUSS_H
#ifdef _WIN32
#pragma once
#endif

struct Concuss_t
{
	int		command;		// concussion command (START, STOP, ETC)
	float	amplitude;		// amplitude (strength) of the blast
};

enum ConcussCommand_t		
{
	CONCUSS_START = 0,		// Starts the concussion effect 
	CONCUSS_STOP,			// Stops the concussion effect
};

extern int gmsgConcuss;		// concussion message

#endif // CONCUSS_H
