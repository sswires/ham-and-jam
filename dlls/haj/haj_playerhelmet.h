//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  HaJ server side helmet entity.
// Notes:    Simple helmet model attached to the players head which falls off sometimes. :)
//
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_HELMET_H
#define HAJ_HELMET_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "props.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPlayerHelmet : public CPhysicsProp
{
public:
	DECLARE_CLASS( CPlayerHelmet, CPhysicsProp );
	DECLARE_SERVERCLASS();

	DECLARE_DATADESC();

	CPlayerHelmet( void );
	~CPlayerHelmet( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );
};

#endif // HAJ_HELMET_H