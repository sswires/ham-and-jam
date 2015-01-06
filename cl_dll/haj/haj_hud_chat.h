//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CS_HUD_CHAT_H
#define CS_HUD_CHAT_H
#ifdef _WIN32
#pragma once
#endif

#include <hud_basechat.h>

class CHudChatLine : public CBaseHudChatLine
{
	DECLARE_CLASS_SIMPLE( CHudChatLine, CBaseHudChatLine );

public:
	CHudChatLine( vgui::Panel *parent, const char *panelName ) : CBaseHudChatLine( parent, panelName ) {}

	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);

	void			PerformFadeout( void );

	void			MsgFunc_SayText(bf_read &msg);

private:
	CHudChatLine( const CHudChatLine & ); // not defined, not accessible

	Color			m_CWChatColor;
	Color			m_AXChatColor;
	Color			m_SPChatColor;
	Color			m_ChatColor;
};

//-----------------------------------------------------------------------------
// Purpose: The prompt and text entry area for chat messages
//-----------------------------------------------------------------------------
class CHudChatInputLine : public CBaseHudChatInputLine
{
	DECLARE_CLASS_SIMPLE( CHudChatInputLine, CBaseHudChatInputLine );
	
public:
	CHudChatInputLine( CBaseHudChat *parent, char const *panelName ) : CBaseHudChatInputLine( parent, panelName ) {}

	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
};

class CHudChat : public CBaseHudChat
{
	DECLARE_CLASS_SIMPLE( CHudChat, CBaseHudChat );

public:
	CHudChat( const char *pElementName );

	virtual void	CreateChatInputLine( void );
	virtual void	CreateChatLines( void );

	virtual void	Init( void );
	virtual void	VidInit( void );
	virtual void	FireGameEvent(IGameEvent *event);
	virtual void	Reset( void );
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	Paint( void );
	virtual void	OnThink( void );

	void			MsgFunc_SayText( bf_read &msg );
	void			MsgFunc_TextMsg( bf_read &msg );
	
	void			ChatPrintf( int iPlayerIndex, const char *fmt, ... );

	int				GetChatInputOffset( void );
	bool			HasVisibleLines( void );
	Color			GetPlayerColor( int clientIndex );

private:

	CHudTexture		*m_pBackground;		// background texture
	bool			bShouldDraw;
	float			m_flBackgroundExpire;
	bool			bFadeBackground;
	
	// background texture
	CPanelAnimationStringVar( 128, m_szBgTexture, "Background", "hud_bg_generic" );

	Color			m_CWChatColor;
	Color			m_AXChatColor;
	Color			m_SPChatColor;
	Color			m_ChatColor;
	
	int				m_origxpos;
	int				m_origypos;
	int				m_xpos;
	int				m_ypos;
	int				m_offset;
};

#endif	//CS_HUD_CHAT_H