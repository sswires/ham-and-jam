//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: The Ham and Jam logo on game menu
// Note:	
// $NoKeywords: $
//=======================================================================//

#include <vgui/VGUI.h>

namespace vgui
{
	class Panel;
}

class ITop
{

public:
	virtual void		Create( vgui::VPANEL parent ) = 0;
	virtual void		Destroy( void ) = 0;

};

extern ITop *Top;