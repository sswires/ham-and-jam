//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  HaJ client side helmet entity.
// Notes:    Simple helmet model attached to the players head which falls off sometimes. :)
//
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_HELMET_C_H
#define HAJ_HELMET_C_H
#ifdef _WIN32
#pragma once
#endif

#include "c_physicsprop.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_PlayerHelmet: public C_PhysicsProp
{
	typedef C_PhysicsProp BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_PlayerHelmet();
	~C_PlayerHelmet();

	virtual bool	ShouldDraw( void );
};

#endif // HAJ_HELMET_C_H