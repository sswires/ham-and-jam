#if !defined( HAJ_FX_TRACERS_H )
#define HAJ_FX_TRACERS_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
#include "particles_simple.h"
#include "c_pixel_visibility.h"

class Vector;

void FX_Player303Tracer( const Vector &start, const Vector &end );
void FX_303Tracer( Vector& start, Vector& end, int velocity, bool makeWhiz );
void FX_Player792Tracer( const Vector &start, const Vector &end );
void FX_792Tracer( Vector& start, Vector& end, int velocity, bool makeWhiz );

#endif // HAJ_FX_TRACERS_H