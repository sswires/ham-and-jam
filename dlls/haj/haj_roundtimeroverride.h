
// haj_roundtimeroverride.h
#include "convar.h"

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_ROUNDTIMEROVERRIDE
#define __INC_ROUNDTIMEROVERRIDE

/////////////////////////////////////////////////////////////////////////////
// includes

extern ConVar haj_attackdefend_roundlimit;

/////////////////////////////////////////////////////////////////////////////
class CRoundTimerOverride
{
public:

	// Changed to "0" for GCC compliance under Linux.
	// Steve, you might want to check code that uses this method to make sure
	// nothing bad happens. - Jed
	//virtual float	GetRoundTimeLeft() = NULL;
	virtual float	GetRoundTimeLeft() = 0;

	virtual void	ExtendRound( float flTime ) { }
	
	virtual int		GetRoundLimit( void )
	{
		return haj_attackdefend_roundlimit.GetInt();
	}

};

/////////////////////////////////////////////////////////////////////////////
#endif
