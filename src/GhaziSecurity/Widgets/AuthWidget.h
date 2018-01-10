#ifndef GS_AUTH_WIDGET_H
#define GS_AUTH_WIDGET_H

#include <Wt/Auth/AuthWidget.h>

namespace GS
{
	class AuthWidget : public Wt::Auth::AuthWidget
	{
	public:
		AuthWidget();

	protected:
		//virtual void createLoginView() override;
	};
}

#endif