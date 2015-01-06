
// haj_player_c.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_PLAYER
#define __INC_PLAYER
#pragma once

/////////////////////////////////////////////////////////////////////////////
// includes
#include "c_hl2mp_player.h"
#include "haj_player_shared.h"
#include "haj_playerhelmet_shared.h"
#include "haj_playerhelmet_c.h"
#include "haj_playeranimstate.h"

class C_BombZone;

#define	WEAPON_MELEE_SLOT			0
#define	WEAPON_PRIMARY_SLOT			1
#define	WEAPON_EXPLOSIVE_SLOT		2
#define	WEAPON_SECONDARY_SLOT		3
#define	WEAPON_TOOL_SLOT			4

/////////////////////////////////////////////////////////////////////////////
class C_HajPlayer : public C_HL2MP_Player
{
public:
	DECLARE_CLASS(C_HajPlayer, C_HL2MP_Player);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

public:
	// 'structors
	C_HajPlayer();
	~C_HajPlayer();

	void			Precache ( void );
	virtual void	ClientThink( void );
	virtual void	AddEntity( void );
	virtual void	Spawn( void );
	virtual void	Weapon_DropPrimary( void );

	virtual const QAngle& GetRenderAngles();
	virtual const QAngle& EyeAngles( void );
	virtual void PostDataUpdate( DataUpdateType_t updateType );

	virtual void	FireBullets( FireBulletsInfo_t &info, bool doLagComp = true );
	virtual bool	ShouldCollide( int collisionGroup, int contentsMask ) const;

	virtual void	HandleBulletPenetration( const FireBulletsInfo_t &info, unsigned short surfaceType, const trace_t &tr, const Vector &vecDir, ITraceFilter *pTraceFilter );
	virtual float	GetMaxPenetrationDistance( unsigned short surfaceType );
	virtual float	CalculatePenetrationDamageLoss( unsigned short surfaceType, float distanceTravelled );

	virtual float	CalcRoll( const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed);

	void			DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual void	UpdateClientSideAnimation();
	void			InitializePoseParams( void );
	virtual CStudioHdr *OnNewModel( void );

	// DoD style recoil
	void			ProcessRecoil();
	void			GetRecoilToAddThisFrame( float &flPitchRecoil, float &flYawRecoil );
	
	float			m_flRecoilTimeRemaining;
	float			m_flPitchRecoilAccumulator;
	float			m_flYawRecoilAccumulator;

	// Clamping of view for prone/deployed states
	virtual void	CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	void			ClampProneAngles( QAngle *vecTestAngles, bool clampX = true, bool clampY = true);
	void			SaveViewAngles();


	// *HAJ 020 - Jed
	// Use for lowering the weapon if not show for a while
	float GetLastShotTime( void ) { return m_flLastShotTime; }
	void  SetLastShotTime( float fTime );

	float			m_flLastShotTime;
	
	QAngle			m_DeployedAngles;
	bool			m_bPlayerIsProne;
	bool			m_bLimitTurnSpeed;
	float			m_flForwardPressedTime;

	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	Vector m_vLookAtTarget;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	void			SetRenderOrigin( Vector vecPos ) { m_vecRenderOrigin = vecPos; }
	virtual const Vector& GetRenderOrigin( void );
	Vector m_vecRenderOrigin;

	bool			CanVault();

	void EnableSprint( bool bEnable);

	virtual int		BloodColor();
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );

	static			C_HajPlayer* GetLocalHajPlayer();
	unsigned int	GetNearbyTeammates() { return clamp( m_nNearbyTeamMates, 0, 3 ); }
	unsigned int	GetNearbyTeammatesUnclamped() { return m_nNearbyTeamMates; } 
	virtual bool	IsWeaponLowered() { return m_bWeaponLowered; };

	unsigned int	GetHelmet() { return m_nHelmetId; }

	virtual	void	DrawTeamSprite();

	float m_fShowHUDAgain;
	float m_fDoJPEGCommandTime;

	bool InDeployZone( void ) { return m_bInDeployZone; }
	int	 DeployZoneStance( void ) { return m_iDeployZStance; }
	int GetObjectiveScore( void ) { return m_iObjectiveScore; }

	bool m_bInBombZone;
	bool IsInBombZone( void ) { return m_bInBombZone; }

	float m_flLastProne;
	CHandle<C_BombZone> m_hBombZone;

	CHajPlayerAnimState* GetAnimState( void ) { return m_pHajAnimState; }

private:
	C_HajPlayer( const C_HajPlayer & );
	unsigned int	m_nNearbyTeamMates;
	bool			m_bWeaponLowered;
	CHandle<CBaseEntity> m_hPickup;
	
	CHandle<CBaseEntity>	m_hHelmetEnt;
	unsigned int			m_nHelmetId;

	CHajPlayerAnimState *m_pHajAnimState;


	bool m_bInDeployZone;
	int m_iDeployZStance;
	int m_iObjectiveScore;
	int m_iObjectivesCapped;
	int m_iObjectivesDefended;

	// *HaJ* Team Icons
	IMaterial			*m_pTeamSprite;
	int					m_iSpriteTeam;
	float				m_flNextSpriteCheck;

};

/////////////////////////////////////////////////////////////////////////////
inline C_HajPlayer* ToHajPlayer(CBaseEntity* pEntity)
{
	if(!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<C_HajPlayer*>(pEntity);
}
/////////////////////////////////////////////////////////////////////////////
#endif