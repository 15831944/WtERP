#ifndef GS_ADMINPAGES_WIDGET_H
#define GS_ADMINPAGES_WIDGET_H

#include <Wt/WTemplate.h>
#include <Wt/WTabWidget.h>

namespace GS
{
	class AdminPageContentWidget;
	class RecordFormView;
	class AbstractFilteredList;

	class AdminPageWidget : public Wt::WTemplate
	{
	public:
		AdminPageWidget(const std::string basePathComponent);

		Wt::WMenuItem *createMenuItemWrapped(std::unique_ptr<RecordFormView> contents);
		Wt::WMenuItem *createMenuItemWrapped(const Wt::WString &label, const std::string &pathComponent, std::unique_ptr<AbstractFilteredList> contents);
		Wt::WMenuItem *createMenuItemWrapped(const Wt::WString &label, const std::string &path, std::unique_ptr<Wt::WWidget> contents, bool isInternalPath);
		Wt::WMenuItem *createMenuItemWrapped(int index, const Wt::WString &label, const std::string &path, std::unique_ptr<Wt::WWidget> contents, bool isInternalPath);
		Wt::WMenuItem *createMenuItem(const Wt::WString &label, const std::string &path, std::unique_ptr<Wt::WWidget> contents, bool isInternalPath);
		Wt::WMenuItem *createMenuItem(int index, const Wt::WString &label, const std::string &path, std::unique_ptr<Wt::WWidget> contents, bool isInternalPath);
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
		AdminPageContentWidget(const Wt::WString &title, std::unique_ptr<Wt::WWidget> content);
		Wt::WText *title() const { return _title; }
		Wt::WWidget *content() const { return _content; }

	protected:
		Wt::WText *_title = nullptr;
		Wt::WWidget *_content = nullptr;
	};

	class DashboardAdminPage : public AdminPageWidget
	{
	public:
		DashboardAdminPage();
	};

	class EntitiesAdminPage : public AdminPageWidget
	{
	public:
		EntitiesAdminPage();

	protected:
		Wt::WMenuItem *_newEntityMenuItem = nullptr;
	};

	class AccountsAdminPage : public AdminPageWidget
	{
	public:
		AccountsAdminPage();
	};

	class AttendanceAdminPage : public AdminPageWidget
	{
	public:
		AttendanceAdminPage();
	};

	class UsersAdminPage : public AdminPageWidget
	{
	public:
		UsersAdminPage();
	};

}

#endif