#include "cbase.h"
#include "haj_magazines.h"
#include "ammodef.h"

/**
	@params:	Ammo type index
*/
CHajWeaponMagazines::CHajWeaponMagazines( CBasePlayer *pPlayerOwner, int iAmmoType )
{
	m_iAmmoType = iAmmoType;
	m_hPlayerOwner = pPlayerOwner;
	m_iCurrentMagazine = 0;

	m_Magazines.EnsureCapacity( GetAmmoDef()->MaxCarry( iAmmoType )+1 ); // reserve memory for our magazines
}

/**
	@returns:	The amount of ammo available for this ammo type
*/
unsigned int CHajWeaponMagazines::RoundCount( void )
{
	unsigned int iCount = 0;
	FOR_EACH_VEC( m_Magazines, i ) iCount += m_Magazines.Element(i);

	return iCount;
}

/**
	@returns:	Amount of ammo available in current magazine
*/
unsigned int CHajWeaponMagazines::CurrentMagazine( void )
{
	if( !m_Magazines.IsValidIndex( m_iCurrentMagazine ) )
		return 0;

	return m_Magazines.Element( m_iCurrentMagazine );
}

/**
	@returns:	Amount of ammo available in magazine we just switch to
*/
unsigned int CHajWeaponMagazines::SwitchToBest( void )
{
	if( !m_Magazines.IsValidIndex(0) )
		return 0;

	int iBestIdx = 0;

	// look for best magazine out of the lot
	for( int i = 1; i < m_Magazines.Count(); i++ )
	{
		// skip if i is current magazine, if not assign if best idx is unassigned or mag has more than our best
		if( i != m_iCurrentMagazine && m_Magazines.IsValidIndex(i) && m_Magazines.Element(i) > m_Magazines.Element(iBestIdx) )
			iBestIdx = i;
	}

	// we found a suitable magazine
	if( m_Magazines.IsValidIndex( iBestIdx ) )
	{
		m_iCurrentMagazine = iBestIdx;
		return m_Magazines.Element(iBestIdx);
	}

	return 0;
}

/**
	@params:	Amount of ammo in current mag
*/
void CHajWeaponMagazines::UpdateCurrent( unsigned int iNewAmount )
{
	if( !m_Magazines.IsValidIndex( m_iCurrentMagazine ) )
		return;

	if( iNewAmount > 0 )
		m_Magazines.Element( m_iCurrentMagazine ) = iNewAmount;
	else
	{
		m_Magazines.Remove( m_iCurrentMagazine );
		m_iCurrentMagazine = max( m_iCurrentMagazine-1, 0 );
	}

	m_hPlayerOwner->SetAmmoCount( MagazineCount()-1, m_iAmmoType );
}

/**
	@params:	Amount of ammo to add to a new magazine
*/
void CHajWeaponMagazines::AddMag( unsigned int iAmount )
{
	if( MagazineCount() > GetAmmoDef()->MaxCarry( m_iAmmoType ) )
		return;

	m_Magazines.AddToTail( iAmount );
	m_hPlayerOwner->SetAmmoCount( MagazineCount()-1, m_iAmmoType );
}

/**
	@params:	Add specified amount of magazines
*/
int CHajWeaponMagazines::AddMags( int iAmount )
{
	Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex( m_iAmmoType );

	if( !pAmmo )
		return 0;

	int iMax = max( iAmount, MagazineCount()-pAmmo->pMaxCarry );

	for( int i = 0; i < iMax; i++ )
		AddMag( pAmmo->iMagazineCapacity );

	return iMax;
}