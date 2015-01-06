
// haj_team.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_TEAM
#define __INC_TEAM

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef TEAM_H
	#include "team.h"
#endif

#include "haj_objectivemanager.h"

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CHajObjective;

/////////////////////////////////////////////////////////////////////////////
class CHajTeam : public CTeam
{
	DECLARE_CLASS(CHajTeam, CTeam);

public:
	// 'structors
	CHajTeam();
	virtual ~CHajTeam();

public:
	static bool IsCombatTeam(int teamId);
	static int GetFirstCombatTeamIndex();

public:
	void RegisterCapturePoint(CHajObjective* pCapturePoint);
	void UnregisterCapturePoint(CHajObjective* pCapturePoint);
	int GetCapturePointsCount() const { return m_capturePoints.Count(); }

	const char* GetSpawnPointClassName() const;
	CBaseEntity* GetNextSpawnPoint(CBasePlayer* pPlayer);

	void ResetRound();

private:
	CUtlVector<CHajObjective*>		m_capturePoints;// list of capture points
	CBaseEntity*					m_pLastSpawnPoint;
};

/////////////////////////////////////////////////////////////////////////////
#endif
