#ifndef HAJ_WEAPON_MAGAZINES_H
#define HAJ_WEAPON_MAGAZINES_H

#include "cbase.h"

class CBasePlayer;

class CHajWeaponMagazines
{
	DECLARE_CLASS_NOBASE( CHajWeaponMagazines );

public:

	CHajWeaponMagazines( CBasePlayer *pPlayerOwner, int iAmmoType );

	void			AddMag( unsigned int iAmount );
	int				AddMags( int iAmount );

	int				MagazineCount( void ) { return m_Magazines.Count(); } // number of magazines (inc. active magazine)
	unsigned int	CurrentMagazine( void ); // get amount of rounds in current magazine
	unsigned int	RoundCount( void ); // number of rounds in total
	unsigned int	SwitchToBest( void );

	void			UpdateCurrent( unsigned int iCurrent ); // update current magazine, deletes if 0
	void			RemoveAll() { m_Magazines.RemoveAll(); m_iCurrentMagazine = 0; }

	int				CurrentMagazineIndex( void ) { return m_iCurrentMagazine; } // current magazine loaded

	CUtlVector<unsigned int> m_Magazines;

private:

	int	m_iCurrentMagazine;
	int m_iAmmoType;

	CHandle<CBasePlayer> m_hPlayerOwner;

};

#endif