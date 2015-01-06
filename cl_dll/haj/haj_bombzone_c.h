
// haj_BombZone.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_BOMBZONE
#define __INC_BOMBZONE

/////////////////////////////////////////////////////////////////////////////
// includes
#include "c_baseentity.h"
#include "haj_objectivemanager_c.h"
#include "c_baseplayer.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"

#undef CBombZone

/////////////////////////////////////////////////////////////////////////////
class C_BombZone : public C_HajObjective
{
public:
	DECLARE_CLASS(C_BombZone, C_HajObjective );
	DECLARE_CLIENTCLASS();

public:
	// 'structors
	C_BombZone();
	~C_BombZone();

public:
	bool operator=(const C_BombZone& rhs) { return m_zoneId == rhs.m_zoneId; }

public:
	// rturns the index of the capturing team
	int GetCapturingTeamId() const { return m_capturingTeam; }

	// returns the capturing percentage
	float GetCapturePercentile() const { return m_capturePercentage; }

	bool IsCWealthLocked() const { return !m_bCWealthCanCapture; }
	bool IsAxisLocked() const { return !m_bAxisCanCapture; }

	bool CanBeCapturedByTeam(int teamId) const;

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

	virtual objectiveroles_e GetTeamRole( int iTeam );
	virtual bool	ShouldDrawAlert();
	virtual void	PostDrawIcon( int x, int y, int iconWidth, int iconHeight, int iconPadding );

	CHandle<CBaseEntity> m_hPlantedBomb;

private:

	// networked data
	int		m_ownerTeam;			// index of the controlling team
	int		m_capturingTeam;
	float	m_capturePercentage;	// range 0.0 to 1.0

	int		m_iBombStage;
	int		m_iStages;
	float	m_flExplodeTime;

	bool	m_bDestroyed;
	bool	m_bPlanted;


	bool	m_bAxisCanCapture; 
	bool	m_bCWealthCanCapture;

	int		m_nMinCWealthPlayers;
	int		m_nMinAxisPlayers;
};

/////////////////////////////////////////////////////////////////////////////
#endif
