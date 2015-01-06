
// haj_capturepoint.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_BOMBZONE
#define __INC_BOMBZONE

/////////////////////////////////////////////////////////////////////////////
// includes
#include "baseentity.h"
#include "utlmap.h"
#include "utllinkedlist.h"
#include "utlvector.h"
#include "haj_objectivemanager.h"


/////////////////////////////////////////////////////////////////////////////
// forward declarations
class CHajTeam;
class CGrenadeTNT;

/////////////////////////////////////////////////////////////////////////////
class CBombZone : public CHajObjective
{
public:
	DECLARE_CLASS(CBombZone, CHajObjective);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	// 'structors
	CBombZone();
	virtual ~CBombZone();

public:
	// CBaseEntity overrides
	void Spawn();
	void Activate();

public:

	bool IsCapturable() const { return true; }
	bool CanBeCapturedByTeam(int teamId) const;

	int GetMinPlayersForCapture(int teamId) const;

	// capturing info
	virtual int GetOwnerTeamId() const { return m_ownerTeam; }
	int GetCapturingTeam( void ) { return m_capturingTeam; }
	float GetCapturePercentage( void ) { return m_capturePercentage; }
	int GetPlayersOnTeam( int team ) { return m_playersInTeam[ team ]; }

	virtual void OnBombExploded( CBasePlayer *pAttacker, CGrenadeTNT *pTNT );
	virtual void OnBombPlant( CBasePlayer *pAttacker, CGrenadeTNT *pTNT );
	virtual void OnBombDefuse( CBasePlayer *pDefuser, CGrenadeTNT *pTNT );

	bool IsBombPlanted() { return m_bPlanted; }

	virtual bool PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info );

	CNetworkHandle( CGrenadeTNT, m_hPlantedBomb );

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
	virtual void OnPlayerLeave( CBasePlayer* pPlayer );

	void Captured(int iCapturingTeam, CBasePlayer *pPlanter );
	void Damaged( int iCapturingTeam, CBasePlayer *pPlanter );

	void DoThink();

private:
	// output events
	COutputEvent m_eOnDestroyed;
	COutputEvent m_eOnDamaged;
	COutputEvent m_eOnAxisDestroy;
	COutputEvent m_eOnCWealthDestroy;
	COutputEvent m_eOnPlanted;
	COutputEvent m_eOnDefused;

	int			m_initialTeam;
	float		m_axisCapTime;
	float		m_cwealthCapTime;
	float		m_fExtraTime;

	CUtlVector<CBasePlayer*> m_capturingPlayers;


	// networked variables
	CNetworkVar(int, m_ownerTeam);		// id of team who controls the zone
	CNetworkVar(int, m_capturingTeam);	// if of the team that is currently capturing the zone
	CNetworkVar(float, m_capturePercentage);
	CNetworkVar(int, m_iStages );
	CNetworkVar(int, m_iBombStage );

	CNetworkVar( bool, m_bDestroyed );
	CNetworkVar( bool, m_bPlanted );
	CNetworkVar( float, m_flExplodeTime );

	CNetworkVar( bool, m_bAxisCanCapture ); 
	CNetworkVar( bool, m_bCWealthCanCapture );

	CNetworkVar(int, m_nMinAxisPlayers);
	CNetworkVar(int, m_nMinCWealthPlayers);
};

/////////////////////////////////////////////////////////////////////////////
#endif
