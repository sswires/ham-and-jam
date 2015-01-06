
// haj_capturepoint.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_CAPTUREPOINT
#define __INC_CAPTUREPOINT

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef BASEENTITY_H
	#include "baseentity.h"
#endif
#ifndef UTLMAP_H
	#include "utlmap.h"
#endif
#ifndef UTLLINKEDLIST_H
	#include "utllinkedlist.h"
#endif
#ifndef UTLVECTOR_H
	#include "utlvector.h"
#endif
#ifndef __INC_OBJECTIVEMANAGER
	#include "haj_objectivemanager.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// defines
#define ENTITY_CAPTURE_POINT			"func_capture_point"

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CHajTeam;

/////////////////////////////////////////////////////////////////////////////
class CCapturePoint : public CHajObjective
{
public:
	DECLARE_CLASS(CCapturePoint, CHajObjective);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	// 'structors
	CCapturePoint();
	virtual ~CCapturePoint();

public:
	// CBaseEntity overrides
	void Spawn();
	void Activate();
	
public:

	bool IsCapturable() const { return true; }
	bool CanBeCapturedByTeam(int teamId) const;

	float GetTeamCaptureTime(int teamId) const;
	int GetMinPlayersForCapture(int teamId) const;

	// capturing info
	virtual int GetOwnerTeamId() const { return m_ownerTeam; }
	int GetCapturingTeam( void ) { return m_capturingTeam; }
	float GetTimeForCapture( void ) { return m_timeLeftForCapture; }
	float GetCapturePercentage( void ) { return m_capturePercentage; }
	int GetPlayersOnTeam( int team ) { return m_playersInTeam[ team ]; }
	float GetExtraTimeForCap( void ) { return m_fExtraTime; }

	virtual bool PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info );

protected:
	virtual void Init();

	// inputs
	void InputEnableAxisCapture(inputdata_t& inputdata);
	void InputDisableAxisCapture(inputdata_t& inputdata);
	
	void InputEnableCWealthCapture(inputdata_t& inputdata);
	void InputDisableCWealthCapture(inputdata_t& inputdata);

private:
	// adds an userId to the list of players in the zone
	virtual void OnPlayerEnter( CBasePlayer* pPlayer );

	void Captured(int iCapturingTeam);

	void DoThink();

	// DoThink helpers
	typedef CUtlMap<int, int>	PlayersPerTeamMap;
	void ProcessOneTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam);
	void ProcessMultipleTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam);
	void ProcessZeroTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam);

private:
	// output events
	COutputEvent m_eOnAxisCapture;		// output event: "OnAxisCapture"
	COutputEvent m_eOnCWealthCapture;	// output event: "OnCWealthCapture"

	float		m_timeLeftForCapture;	// amount of time left until capture
	int			m_initialTeam;
	float		m_axisCapTime;
	float		m_cwealthCapTime;
	int			m_counterStyle;
	int			m_nCounterPlayers;
	float		m_fExtraPeopleCap; // cap speed for extra players (percentile)
	float		m_fExtraTime;
	float		m_flDeCapSpeed;

	CUtlVector<CHajPlayer*> m_CapInitialisers;

	// networked variables
	CNetworkVar(int, m_ownerTeam);		// id of team who controls the zone
	CNetworkVar(int, m_capturingTeam);	// if of the team that is currently capturing the zone
	CNetworkVar(float, m_capturePercentage);
	
	CNetworkVar( bool, m_bAxisCanCapture ); 
	CNetworkVar( bool, m_bCWealthCanCapture );

	CNetworkVar(int, m_nMinAxisPlayers);
	CNetworkVar(int, m_nMinCWealthPlayers);
};

/////////////////////////////////////////////////////////////////////////////
#endif
