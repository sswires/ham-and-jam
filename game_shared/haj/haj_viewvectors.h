
// haj_viewvectors.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_VIEWVECTORS
#define __INC_VIEWVECTORS

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef SHAREDDEFS_H
	#include "shareddefs.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CHajViewVectors : public CViewVectors
{
public:
	CHajViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vDuckViewMove,
		Vector vProneHullMin,
		Vector vProneHullMax,
		Vector vProneView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vCrouchTraceMin,
		Vector vCrouchTraceMax );

	Vector m_vCrouchTraceMin;
	Vector m_vCrouchTraceMax;	
};

/////////////////////////////////////////////////////////////////////////////
// globals
extern CHajViewVectors _hajViewVectors;

/////////////////////////////////////////////////////////////////////////////
#endif
