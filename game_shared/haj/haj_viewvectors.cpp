
// haj_viewvectors.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_viewvectors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
CHajViewVectors::CHajViewVectors( 
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
		Vector vCrouchTraceMax ) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vProneHullMin,
				vProneHullMax,
				vProneView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
{
	m_vCrouchTraceMin = vCrouchTraceMin;
	m_vCrouchTraceMax = vCrouchTraceMax;
	m_vDuckViewMove = vDuckViewMove;
}

/////////////////////////////////////////////////////////////////////////////
CHajViewVectors _hajViewVectors(
	Vector( 0, 0, 66 ),       //m_vView
							  
	Vector(-16, -16, 0 ),	  //m_vHullMin
	Vector( 16,  16,  72 ),	  //m_vHullMax
							  
	Vector(-16, -16, 0 ),	  //m_vDuckHullMin
	Vector( 16,  16, 52 ),	  //m_vDuckHullMax
	Vector( 0, 0, 42 ),		  //m_vDuckView
	Vector( 0, 0, 52 ),		 // m_vDuckViewMove

	Vector(-16, -16, 0 ),	  //m_vProneHullMin	
	Vector( 16,  16,  24 ),	  //m_vProneHullMin
	Vector( 0, 0, 12 ),		  //m_vProneView
							  
	Vector(-10, -10, -10 ),	  //m_vObsHullMin
	Vector( 10,  10,  10 ),	  //m_vObsHullMax
							  
	Vector( 0, 0, 66 ),		  //m_vDeadViewHeight

	Vector(-16, -16, 0 ),	  //m_vCrouchTraceMin
	Vector( 16,  16,  60 )	  //m_vCrouchTraceMax
);

/////////////////////////////////////////////////////////////////////////////
