//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World Stielhandgranate class
//
// $NoKeywords: $
//=======================================================================//

#include "basegrenade_shared.h"

#ifndef GRENADE_STIEL_H
#define GRENADE_STIEL_H
#pragma once

CBaseGrenade *StielGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

#endif // GRENADE_STIEL_H
