#include "Widgets/AuthWidget.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

namespace ERP
{

	AuthWidget::AuthWidget()
		: Wt::Auth::AuthWidget(SERVER->getAuthService(), APP->userDatabase(), APP->authLogin())
	{
		WServer *server = SERVER;

		model()->addPasswordAuth(&server->getPasswordService());
		for(const auto &oauthPtr : server->getOAuthServices())
			model()->addOAuth(oauthPtr.get());

		setRegistrationEnabled(false);
		processEnvironment();
	}

}
