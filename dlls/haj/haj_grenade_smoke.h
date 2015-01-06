//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World smoke grenade class
//
// $NoKeywords: $
//=======================================================================//

#ifndef GRENADE_SMOKE_H
#define GRENADE_SMOKE_H
#pragma once

class CHAJGrenade;
struct edict_t;

CHAJGrenade *SmokeGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, float duration, const char *pszModelname );
CHAJGrenade *SmokeGrenadeNo77_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float smoke_damage, float duration, const char *pszModelname );

#endif // GRENADE_SMOKE_H
