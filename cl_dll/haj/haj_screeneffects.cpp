//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Handles HaJ screen overlay effects sent via network messages
// Note:	Similar to env_overlay in implementation but requires no entity
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "shareddefs.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "texture_group_names.h"
#include "vstdlib/icommandline.h"
#include "keyvalues.h"
#include "ScreenSpaceEffects.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "concuss.h"
#include "hud_macros.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// class definition
class C_HaJScreenEffects
{

public:
	
	C_HaJScreenEffects();
	
	void ReceiveMessage( int cmd, float amplitude );

private:
};

static C_HaJScreenEffects gHaJScreenEffects;

//-------------------------------------------------
// Message hook function for the concussion effect
//-------------------------------------------------
void __MsgFunc_Concuss( bf_read &msg )
{
	Concuss_t concuss;

	concuss.command	= msg.ReadByte();
	concuss.amplitude = msg.ReadFloat();
	
	gHaJScreenEffects.ReceiveMessage( concuss.command, concuss.amplitude );
}

//-------------------------------------------------
// Construction
//-------------------------------------------------
C_HaJScreenEffects::C_HaJScreenEffects( void )
{
	// hook our concuss message
	HOOK_MESSAGE( Concuss );
}


//-------------------------------------------------
// Recieve our concuss messsage data
//-------------------------------------------------
void C_HaJScreenEffects::ReceiveMessage( int cmd, float amplitude )
{

	KeyValues *pKeys = new KeyValues( "keys" );
	if ( pKeys == NULL ) { return; }

	if( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80 ) { return; }

	// Just use the episodic stun effect for now
	switch ( cmd )
	{
	case CONCUSS_START:
	
		pKeys->SetFloat( "duration", amplitude * 0.08f );

		g_pScreenSpaceEffects->SetScreenSpaceEffectParams( "episodic_stun", pKeys );
		g_pScreenSpaceEffects->EnableScreenSpaceEffect( "episodic_stun" );
	
		break;
	
	case CONCUSS_STOP:
	default:

		g_pScreenSpaceEffects->DisableScreenSpaceEffect( "episodic_stun" );
		break;
	}
}