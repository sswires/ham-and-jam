
// haj_objectivemanager.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_OBJECTIVEMANAGER
#define __INC_OBJECTIVEMANAGER

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLLINKEDLIST_H
	#include "utllinkedlist.h"
#endif

#include "cbase.h"
#include "haj_gamerules.h"

/////////////////////////////////////////////////////////////////////////////
// forward declarations
class C_CapturePoint;

/////////////////////////////////////////////////////////////////////////////
class C_HajObjective : public C_BaseEntity
{
	DECLARE_CLASS(C_HajObjective, C_BaseEntity);
	DECLARE_CLIENTCLASS();

public:

	C_HajObjective() {}
	~C_HajObjective() {}

	// C_BaseEntity overrides
	virtual void ClientThink();
	virtual void Spawn();

	const Vector& GetAbsOrigin() const;

	virtual int		GetSortIndex() { return m_sortIndex; }

	// stuff that should be overridden
	virtual int		GetOwnerTeamId() const { return TEAM_INVALID; }
	virtual int		GetCapturingTeamId() const { return TEAM_INVALID; }
	virtual float	GetCapturePercentile() const { return 0; }
	virtual bool	IsLocalPlayerInArea( void ) { return m_bLocalPlayerInArea; };
	virtual int		GetOccupantsByTeam( int teamid );

	// HUD stuff
	virtual bool	GetShowOnHud() { return m_bShowOnHud; } // accessor for the show on hud variable
	virtual const char* GetTeamIcon( int iTeamID );
	virtual char	*GetAxisIcon() const { return (char *)&m_szIconAxis; };
	virtual char	*GetAlliesIcon() const { return (char *)&m_szIconAllies; };
	virtual char	*GetNeutralIcon() const { return (char *)&m_szIconNeutral; };
	char			*GetNameOfZone() const { return (char *)&m_nameOfZone; };

	virtual void	PostDrawIcon( int x, int y, int iconWidth, int iconHeight, int iconPadding ) {}
	virtual void	PostDrawInfoBar( int x, int y, int w, int h ) {};

	virtual bool	IsLockedForTeam( int iTeam ) { return false; }

	// Events
	virtual void	OnLocalPlayerEnter( void ) {}; // remember these are on the client
	virtual void	OnLocalPlayerLeave( void ) {};

	virtual objectiveroles_e GetTeamRole( int iTeam ) { return OBJECTIVE_ROLE_UNKNOWN; }
	objectiveroles_e GetEnemyTeamRole( int iTeam )
	{
		if( iTeam == TEAM_AXIS )
			return GetTeamRole( TEAM_CWEALTH );

		return GetTeamRole( TEAM_AXIS );
	}

	virtual bool	ShouldDrawAlert() { return false; }

	virtual const char* GetStringForRole( void );

	virtual int GetCapCountByTeam( int iTeam )	{ return 1; }

	int		m_zoneId;				// id of the control point
	bool	m_bShowOnHud;			// show capture point on Hud
	int		m_sortIndex;

	char	m_szIconNeutral[MAX_PATH];
	char	m_szIconAllies[MAX_PATH];
	char	m_szIconAxis[MAX_PATH];
	char	m_nameOfZone[MAX_PATH];

	EHANDLE	m_playersInArea[32];

private:
	// private data
	bool	m_bLocalPlayerInArea;

};

/////////////////////////////////////////////////////////////////////////////
class C_HajObjectiveManager
{
public:
	// adds a capture point objective
	void AddCapturePoint(C_CapturePoint* pCapturePoint);

	// removes a capture point objective
	void RemoveCapturePoint(C_CapturePoint* pCapturePoint);

	// returns a list of all capture point objectives
	const CUtlLinkedList<C_CapturePoint*>& GetCapturePoints() const { return m_capturePoints; }

	// returns a list of all objectives
	const CUtlLinkedList<C_HajObjective*>& GetObjectives() const { return m_objectives; }

	void AddObjective(C_HajObjective* pObjective);
	void RemoveObjective(C_HajObjective* pObjective);

private:
	// private data
	CUtlLinkedList<C_HajObjective*> m_objectives;
	CUtlLinkedList<C_CapturePoint*> m_capturePoints;
};

/////////////////////////////////////////////////////////////////////////////
extern C_HajObjectiveManager _objectiveman;

/////////////////////////////////////////////////////////////////////////////
#endif
