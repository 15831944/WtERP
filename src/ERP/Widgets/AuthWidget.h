#ifndef ERP_AUTH_WIDGET_H
#define ERP_AUTH_WIDGET_H

#include "Common.h"
#include <Wt/Auth/AuthWidget.h>

namespace ERP
{
	class AuthWidget : public Wt::Auth::AuthWidget
	{
	public:
		AuthWidget();
	};
}

#endif