
// haj_capturepoint.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_CAPTUREPOINT
#define __INC_CAPTUREPOINT

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef C_BASEENTITY_H
	#include "c_baseentity.h"
#endif
#ifndef __INC_OBJECTIVEMANAGER
	#include "haj_objectivemanager_c.h"
#endif

#include "c_baseplayer.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"

/////////////////////////////////////////////////////////////////////////////
class C_CapturePoint : public C_HajObjective
{
public:
	DECLARE_CLASS(C_CapturePoint, C_HajObjective );
	DECLARE_CLIENTCLASS();

public:
	// 'structors
	C_CapturePoint();
	~C_CapturePoint();

public:
	bool operator=(const C_CapturePoint& rhs) { return m_zoneId == rhs.m_zoneId; }

public:
	// rturns the index of the capturing team
	int GetCapturingTeamId() const { return m_capturingTeam; }

	// returns the capturing percentage
	float GetCapturePercentile() const { return m_capturePercentage; }

	bool IsCWealthLocked() const { return !m_bCWealthCanCapture; }
	bool IsAxisLocked() const { return !m_bAxisCanCapture; }

	bool IsLockedForTeam( int iTeam )
	{
		if( iTeam == TEAM_CWEALTH )
			return IsCWealthLocked();
		else if( iTeam == TEAM_AXIS )
			return IsAxisLocked();

		return false;
	}

public:
	// C_HajObjective implementation
	// returns the index of the controlling team
	int GetOwnerTeamId() const { return m_ownerTeam; }

	int GetAlliesCapCount() const { return m_nMinCWealthPlayers; }
	int GetAxisCapCount() const { return m_nMinAxisPlayers; }

	int GetCapCountByTeam( int iTeam )
	{
		if( iTeam == TEAM_AXIS )
			return GetAxisCapCount();
		else if( iTeam == TEAM_CWEALTH )
			return GetAlliesCapCount();

		return -1;
	}

	virtual objectiveroles_e GetTeamRole( int iTeam )
	{
		if( GetOwnerTeamId() == iTeam && !( ( iTeam == TEAM_CWEALTH && IsAxisLocked() ) || ( iTeam == TEAM_AXIS && IsCWealthLocked() ) ) )
			return OBJECTIVE_ROLE_DEFEND;
		else if( ( iTeam == TEAM_CWEALTH && IsCWealthLocked() ) || ( iTeam == TEAM_AXIS && IsAxisLocked() ) )
			return OBJECTIVE_ROLE_LOCKED;
		else if( GetOwnerTeamId() == iTeam )
			return OBJECTIVE_ROLE_COMPLETED;

		return OBJECTIVE_ROLE_CAPTURE;

	}

	virtual bool	ShouldDrawAlert();
	virtual void	PostDrawIcon( int x, int y, int iconWidth, int iconHeight, int iconPadding );

private:

	// networked data
	int		m_ownerTeam;			// index of the controlling team
	int		m_capturingTeam;
	float	m_capturePercentage;	// range 0.0 to 1.0

	bool	m_bAxisCanCapture; 
	bool	m_bCWealthCanCapture;

	int		m_nMinCWealthPlayers;
	int		m_nMinAxisPlayers;
};

/////////////////////////////////////////////////////////////////////////////
#endif
