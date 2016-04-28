#ifndef GS_AUTH_WIDGET_H
#define GS_AUTH_WIDGET_H

#include <Wt/Auth/AuthWidget>

namespace GS
{
	class AuthWidget : public Wt::Auth::AuthWidget
	{
	public:
		AuthWidget(Wt::WContainerWidget *parent = nullptr);

	protected:
		//virtual void createLoginView() override;
	};
}

#endif