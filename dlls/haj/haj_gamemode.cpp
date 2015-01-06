#include "cbase.h"
#include "haj_mapsettings_enums.h"
#include "haj_gamerules.h"
#include "haj_gamemode.h"

BEGIN_DATADESC( CHajGameMode )

DEFINE_KEYFIELD(m_soundCWealthVictory, FIELD_STRING,	"CWealthVictorySound"),
	DEFINE_KEYFIELD(m_soundAxisVictory,    FIELD_STRING,	"AxisVictorySound"),
	DEFINE_KEYFIELD(m_soundStalemate,		FIELD_STRING,	"StalemateSound"),
	DEFINE_KEYFIELD( m_bMusicOverride,		FIELD_BOOLEAN, "OverrideMusic" ),
	DEFINE_OUTPUT( m_eOnCWealthWin, "OnCwealthWin"),
	DEFINE_OUTPUT( m_eOnAxisWin, "OnAxisWin" ),
	DEFINE_OUTPUT( m_eOnRoundEnd, "OnRoundEnd" ), 
	DEFINE_OUTPUT( m_eOnRoundReset, "OnRoundReset" ),

	DEFINE_OUTPUT( m_eOnGameEndScreen, "OnGameEnd" ),
	DEFINE_OUTPUT( m_eOnAxisGameWin, "OnAxisGameWin" ),
	DEFINE_OUTPUT( m_eOnCWealthGameWin, "OnCwealthGameWin" ),
	DEFINE_OUTPUT( m_eOnGameDraw, "OnGameDraw" ),
END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED( HajGameMode, DT_HajGameMode )

BEGIN_NETWORK_TABLE( CHajGameMode, DT_HajGameMode )
END_NETWORK_TABLE()

void CHajGameMode::Spawn()
{
	SetGametype();

	m_eOnRoundReset.FireOutput( NULL, NULL );
}

void CHajGameMode::SetGametype( void )
{
	CHajGameRules* pGameRules = HajGameRules();
	pGameRules->SetGametype( GetBasicGameType() );
	pGameRules->SetGamemode( this );
}


bool CHajGameMode::ShouldFreezeTime( void )
{
	CHajGameRules *pRules = HajGameRules();

	if( pRules )
	{
		if( pRules->IsFreeplay() || pRules->IsFreezeTime() )
			return true;
	}

	return false;
}

// attack/defend or push/pull structure?
int CHajGameMode::GetBasicGameType( void )
{
	return HAJ_GAMEMODE_ATTACKDEFEND;
}

void CHajGameMode::Precache()
{
	PrecacheSound( "Game.BritishRoundWin" );
	PrecacheSound( "Game.CanadianRoundWin" );
	PrecacheSound( "Game.PolishRoundWin" );
	PrecacheSound( "Game.GermanRoundWin" );
	PrecacheSound( "Game.StalemateRoundResult" );
}

void CHajGameMode::PlayVictorySound( int winningTeamId )
{
	if( m_bMusicOverride )
	{
		PlayCustomVictoryMusic( winningTeamId );
	}
	else
	{
		CHajGameRules *pGameRules = HajGameRules();
		const char* szSoundName = NULL;

		if( pGameRules )
		{
			int iCWealthNation = pGameRules->GetCommonwealthNation();

			switch( winningTeamId )
			{
			case TEAM_CWEALTH:
				switch( iCWealthNation )
				{
					case NATION_CWEALTH_BRITAIN:
						szSoundName = "Game.BritishRoundWin";
						break;

					case NATION_CWEALTH_CANADA:
						szSoundName = "Game.CanadianRoundWin";
						break;

					case NATION_CWEALTH_POLAND:
						szSoundName = "Game.PolishRoundWin";
						break;

					default:
						szSoundName = "Game.BritishRoundWin";
				}
				break;

			case TEAM_AXIS:
				szSoundName = "Game.GermanRoundWin";
				break;
				
			default:
				szSoundName = "Game.StalemateRoundResult";
			}
		}

		if( !szSoundName || szSoundName[0] == '\0' )
			return;

		IGameEvent* pEvent = gameeventmanager->CreateEvent( "global_sound" );

		if( pEvent )
		{
			pEvent->SetString( "sound", szSoundName );
			gameeventmanager->FireEvent( pEvent );
		}
	}
}

void CHajGameMode::PlayCustomVictoryMusic( int winningTeamId )
{
	const char* szSoundName = NULL;

	switch(winningTeamId)
	{
	case -1: // stalemate
		if( m_soundStalemate != MAKE_STRING( "" ) && m_soundStalemate.ToCStr())
			szSoundName = m_soundStalemate.ToCStr();

		break;
	case TEAM_CWEALTH:
		szSoundName = m_soundCWealthVictory.ToCStr();
		break;

	case TEAM_AXIS:
		szSoundName = m_soundAxisVictory.ToCStr();
		break;
	}

	if(szSoundName != NULL)
	{
		CBroadcastRecipientFilter filter;
		enginesound->EmitSound(filter, -1, CHAN_STATIC, szSoundName, 1.0, 0.0f);
	}
}

void CHajGameMode::PlayerSpawn( CHajPlayer *pPlayer )
{

}
