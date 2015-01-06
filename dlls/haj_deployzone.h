// haj_deployzone.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_DEPLOYZONE
#define __INC_DEPLOYZONE

class CDeployZone: public CBaseTrigger
{
public:
	DECLARE_CLASS( CDeployZone, CBaseTrigger );
	DECLARE_DATADESC();

	void Spawn();
	void StartTouch(CBaseEntity *pOther);
	void EndTouch(CBaseEntity *pOther);

	int GetStance() { return m_iStance; }
	int GetTeamFilter() { return m_iTeamFilter; }
	float GetAngleTolerance() { return m_fAngleMax; }

private:
	int m_iStance, m_iTeamFilter;
	float m_fAngleMax, m_fMinYaw, m_fMaxYaw;
};

#endif