//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose: Proxy for level loading textures
// Note:	
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "materialsystem/IMaterialProxy.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/ITexture.h"
#include "filesystem.h"
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define		DEFAULT_NAME	"vgui/loading/default"
#define		MAXTIPS			3
#define		TIPSHOWTIME		10

extern IFileSystem *filesystem;
class CLevelLoadingProxy : public IMaterialProxy
{
public:
	CLevelLoadingProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void );

	IMaterialVar	*m_pBaseTextureVar;		// variable for our base texture
	ITexture		*m_pDefaultTexture;		// default texture
	
	int				nTips;
	int				nLastTip;
	int				nCurrentTip;

	float			flNextChangeTime;		// last time we changed the tip
};

CLevelLoadingProxy::CLevelLoadingProxy()
{
	m_pBaseTextureVar = NULL;
	m_pDefaultTexture = NULL;
	nLastTip = 0;
	nCurrentTip = 0;
	nTips = 0;
	flNextChangeTime = 0.0f;
}

void CLevelLoadingProxy::Release()
{
	m_pBaseTextureVar = NULL;
	m_pDefaultTexture = NULL;
	delete this;
}

bool CLevelLoadingProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool found = false;
	
	// check for $basetexture parameter
	m_pBaseTextureVar = pMaterial->FindVar( "$basetexture", &found );
    
	if ( !found )
		return false;

	// find the default texture
	m_pDefaultTexture = materials->FindTexture( DEFAULT_NAME, TEXTURE_GROUP_VGUI );

	// check the above isn't an error texture
	if ( IsErrorTexture( m_pDefaultTexture ) )
		return false;
	
	m_pDefaultTexture->IncrementReferenceCount();

	char curFile[200];
	sprintf( curFile, "materials/vgui/loading/gametip%d.vtf", nTips + 1 );

	while( filesystem->FileExists( curFile ) )
	{
		nTips++;
		sprintf( curFile, "materials/vgui/loading/gametip%d.vtf", nTips + 1 );
	}
	
	flNextChangeTime = 0.0f;
	return true;
}

void CLevelLoadingProxy::OnBind( void *pC_BaseEntity )
{
	// bail if no base variable
	if ( !m_pBaseTextureVar )
		return;

	ITexture *texture = NULL;
	char loadingimage[512];

	/*
	char szMapname[512] = {0x00};
	
	// try and get the name of our map from the server
	Q_FileBase( engine->GetLevelName(), szMapname, sizeof( szMapname) );

	// if we got a map name
	if ( szMapname[512] != 0x00 )
	{
		// make it into a path
		Q_snprintf( loadingimage, sizeof( loadingimage ), "vgui/loading/%s", szMapname );
		
		// try and load it
		texture = materials->FindTexture( loadingimage, TEXTURE_GROUP_VGUI, false );
		
		if ( !IsErrorTexture( texture ) )
		{
			m_pBaseTextureVar->SetTextureValue( texture );
			flNextChangeTime = engine->Time() + 50.0f;
			return;
		}
	}
	*/
	// no specific map texture found or show a tip or a default. //
	
	// check we have a default
	if ( m_pDefaultTexture )
	{
		// if we're due to change tip.
		if ( flNextChangeTime <= engine->Time() )
		{
			nLastTip = nCurrentTip;

			while ( nLastTip == nCurrentTip )
				nCurrentTip = RandomInt( 1, nTips );
			
			Q_snprintf( loadingimage, sizeof( loadingimage ), "vgui/loading/gametip%d", nCurrentTip );
			texture = materials->FindTexture( loadingimage, TEXTURE_GROUP_VGUI, false );
			
			if ( !IsErrorTexture( texture ) )
			{
				m_pBaseTextureVar->SetTextureValue( texture );
				flNextChangeTime = engine->Time() + 10.0f;
				return;
			}
			else
			{
				m_pBaseTextureVar->SetTextureValue( m_pDefaultTexture );
			}
		}
	}
}

EXPOSE_INTERFACE( CLevelLoadingProxy, IMaterialProxy, "LevelLoading" IMATERIAL_PROXY_INTERFACE_VERSION );
