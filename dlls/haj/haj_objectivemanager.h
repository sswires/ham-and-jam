
// haj_objectivemanager.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_OBJECTIVEMANAGER
#define __INC_OBJECTIVEMANAGER

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamerules.h"
#include "haj_team.h"
#include "utllinkedlist.h"

/////////////////////////////////////////////////////////////////////////////
// defines
#define CAPTUREPOINT_UPDATE_INTERVAL		0.1f

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CCapturePoint;
class CHajTeam;
class CHajPlayer;

/////////////////////////////////////////////////////////////////////////////
class CHajObjective : public CBaseToggle
{
	DECLARE_CLASS( CHajObjective, CBaseToggle );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:

	CHajObjective();
	~CHajObjective();

	virtual void Spawn();
	virtual void Activate();
	virtual void Init();
	const Vector& GetAbsOrigin() const;

	// Think functions
	virtual void BrushThink( void );
	virtual void DoThink( void ) {}; // to be overridden

	// always transmit over network
	int UpdateTransmitState() {	return SetTransmitState(FL_EDICT_ALWAYS); }

	// adds an userId to the list of players in the zone
	bool IsPlayerInList(int userId);
	bool AddPlayer(CBasePlayer* pPlayer);
	void RemovePlayer( CBasePlayer* pPlayer );
	virtual void OnPlayerEnter( CBasePlayer* pPlayer ) {};
	virtual void OnPlayerLeave( CBasePlayer* pPlayer ) {};

	// called when an entity enters and exits the trigger
	virtual void StartTouch(CBaseEntity* pOther);
	virtual void EndTouch(CBaseEntity* pOther);

	// register/unregister with a team
	void RegisterWithTeam();
	void UnregisterWithTeam();

	virtual bool PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info );

	// stuff that needs overriding
	virtual int GetOwnerTeamId() const { return TEAM_INVALID; }
	CHajTeam* GetOwnerTeam() const;
	virtual bool CanBeCapturedByTeam( int iTeam ) { return false; } // must override

	string_t GetNameOfZone() const { return m_nameOfZone; }

	int GetZoneId() const { return m_zoneId; }

	// comparision function for the map data structure
	// returns true if the first param is less-than the second param
	static bool LessFunc(const int& a, const int& b) { return (a < b); }

	// 
	CNetworkVar(int, m_sortIndex);
	CNetworkVar(int, m_zoneId);			// id of zone
	CNetworkVar( string_t, m_nameOfZone );	// name of zone. display on screen when captured
	int			m_zoneVisible;			// indicates if zone is visible

	// HUD Icons
	CNetworkVar( string_t, m_szIconNeutral );
	CNetworkVar( string_t, m_szIconAllies );
	CNetworkVar( string_t, m_szIconAxis );
	CNetworkVar( bool, m_bShowOnHud );	// show element on HUD

	// Players in area
	CNetworkArray(EHANDLE, m_playersInArea, 32);
	CUtlMap<int, int>	m_playerList;	// list of players within the zone (key = userid, value = userid)
	CUtlVector<int> m_playersInTeam;


	float		m_lastThinkTime;		// time this entity last recieved the Think event
	float		m_clientUpdateInterval;
};

/////////////////////////////////////////////////////////////////////////////
class CHajObjectiveManager
{
public:
	// 'structors
	CHajObjectiveManager();
	~CHajObjectiveManager();

public:
	// adds a capture point objective
	void AddCapturePoint(CCapturePoint* pCapturePoint);
	void AddObjective(CHajObjective* pObjective);

	// removes a capture point objective
	void RemoveCapturePoint(CCapturePoint* pCapturePoint);
	void RemoveObjective(CHajObjective* pObjective);

	// returns the number of capture points
	int GetNumCapturePoints() const { return m_capturePoints.Count(); }

	// returns the total number of objectives
	int GetNumObjectives() const { return m_objectives.Count(); }

	// returns a list of all capture point objectives
	const CUtlLinkedList<CCapturePoint*>& GetCapturePoints() const { return m_capturePoints; }

	// returns a list of all objectives
	const CUtlLinkedList<CHajObjective*>& GetObjectives() const { return m_objectives; }

	// returns true if the specified team has control of all objectives
	bool HasTeamCompletedAllObjectives(int teamId) const;

	// return true if the specified team has captured all control points
	bool HasTeamCapturedAllControlPoints(int teamId) const;

private:
	// private data
	CUtlLinkedList<CHajObjective*> m_objectives;
	CUtlLinkedList<CCapturePoint*> m_capturePoints;
};

/////////////////////////////////////////////////////////////////////////////
extern CHajObjectiveManager _objectiveman;

/////////////////////////////////////////////////////////////////////////////
#endif
