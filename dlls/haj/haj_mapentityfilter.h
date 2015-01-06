
// haj_mapentityftler.h

#ifndef __INC_MAPENTITYFILTER
#define __INC_MAPENTITYFILTER

/////////////////////////////////////////////////////////////////////////////
// includes
#include "mapentities.h"
#include "utlhash.h"

/////////////////////////////////////////////////////////////////////////////
class CMapEntityFilter : public IMapEntityFilter
{
public:
	// 'structors
	CMapEntityFilter();
	~CMapEntityFilter();

public:
	void AddFilter(const char* szFilter);
	void ClearFilters();

public:
	// IMapEntityFilter overrides
	CBaseEntity* CreateNextEntity(const char *pClassname);
	bool ShouldCreateEntity(const char *pClassname);

	// Hashtable callbacks
	static bool CompareFunc(const long& a, const long& b);
	static unsigned int KeyFunc(const long& item);

private:
	long ComputeChecksum(const char* szBuffer);

private:
	CUtlHash<long> m_filter;
};

/////////////////////////////////////////////////////////////////////////////
#endif
