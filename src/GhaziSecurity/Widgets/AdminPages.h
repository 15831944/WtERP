#ifndef GS_ADMINPAGES_WIDGET_H
#define GS_ADMINPAGES_WIDGET_H

#include "Widgets/EntityView.h"

#include <Wt/WTemplate>
#include <Wt/WTabWidget>

namespace GS
{
	class AdminPageContentWidget;

	class AdminPageWidget : public Wt::WTemplate
	{
	public:
		AdminPageWidget(const std::string basePathComponent, Wt::WContainerWidget *parent = nullptr);

		Wt::WMenuItem *createMenuItemWrapped(const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents);
		Wt::WMenuItem *createMenuItemWrapped(int index, const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents);
		Wt::WMenuItem *createMenuItem(const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents);
		Wt::WMenuItem *createMenuItem(int index, const Wt::WString &label, const std::string &internalPath, Wt::WWidget *contents);
		bool checkPathComponentExist(const std::string &pathComponent) const;
		void connectFormSubmitted(Wt::WMenuItem *item);
		void setDeniedPermissionWidget();

		const std::string &basePathComponent() const { return _basePathComponent; }
		Wt::WNavigationBar *sideBar() const { return _sideBar; }
		Wt::WStackedWidget *stackWidget() const { return _stackWidget; }
		Wt::WMenu *menu() const { return _menu; }
		static Wt::WWidget *itemContent(Wt::WMenuItem *item);

	protected:
		//void handleItemSelected(Wt::WMenuItem *item);
		void handleFormSubmitted(Wt::WMenuItem *item);

		Wt::WNavigationBar *_sideBar = nullptr;
		Wt::WStackedWidget *_stackWidget = nullptr;
		Wt::WMenu *_menu = nullptr;
		Wt::WTemplate *_deniedPermissionWidget = nullptr;
		std::string _basePathComponent;

		typedef std::map<Wt::WMenuItem*, Wt::Signals::connection> SignalMap;
		SignalMap _submitSignalMap;
	};

	class AdminPageContentWidget : public Wt::WTemplate
	{
	public:
		AdminPageContentWidget(const Wt::WString &title, Wt::WWidget *content, Wt::WContainerWidget *parent = nullptr);
		Wt::WText *title() const { return _title; }
		Wt::WWidget *content() const { return _content; }

	protected:
		Wt::WText *_title = nullptr;
		Wt::WWidget *_content = nullptr;
	};

	class EntitiesAdminPage : public AdminPageWidget
	{
	public:
		EntitiesAdminPage(Wt::WContainerWidget *parent = nullptr);

	protected:
		Wt::WMenuItem *_newEntityMenuItem = nullptr;
	};

	class AccountsAdminPage : public AdminPageWidget
	{
	public:
		AccountsAdminPage(Wt::WContainerWidget *parent = nullptr);
	};

}

#endif