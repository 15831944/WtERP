#ifndef GS_ATTENDANCEDEVICE_MVC_H
#define GS_ATTENDANCEDEVICE_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/RecordFormView.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>

namespace GS
{
	class AttendanceDeviceView;
	class AttendanceEntryView;

	//ATTENDANCE DEVICE
	class AttendanceDeviceListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AttendanceDeviceListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AttendanceDeviceList : public QueryModelFilteredList<boost::tuple<long long, std::string, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResHostName, ResCountryName, ResCityName, ResAddress };
		enum ViewColumns { ViewId, ViewHostName, ViewCountry, ViewCity, ViewAddress };

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};

	class AttendanceDeviceFormModel : public RecordFormModel<AttendanceDevice>
	{
	public:
		static const Wt::WFormModel::Field hostNameField;
		static const Wt::WFormModel::Field locationField;

		AttendanceDeviceFormModel(AttendanceDeviceView *view, Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr = Wt::Dbo::ptr<AttendanceDevice>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		AttendanceDeviceView *_view = nullptr;
	};

	class AttendanceDeviceView : public RecordFormView
	{
	public:
		AttendanceDeviceView(Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr = Wt::Dbo::ptr<AttendanceDevice>());
		virtual void initView() override;

		Wt::Dbo::ptr<AttendanceDevice> attendanceDevicePtr() const { return _model->recordPtr(); }
		AttendanceDeviceFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override { return tr("AttendanceDeviceViewName").arg(attendanceDevicePtr().id()); }
		virtual std::string viewInternalPath() const override { return attendanceDevicePtr() ? AttendanceDevice::viewInternalPath(attendanceDevicePtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new AttendanceDeviceView(); }

	protected:
		AttendanceDeviceFormModel *_model = nullptr;
		Wt::Dbo::ptr<AttendanceDevice> _tempPtr;
	};

	//ATTENDANCE ENTRY
	class AttendanceEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AttendanceEntryListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &idx, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AttendanceEntryList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WDateTime, Wt::WDateTime, std::string, std::string, std::string>>
	{
	public:
		enum ResultColumns { ResId, ResEntityName, ResTimestampIn, ResTimestampOut, ResCountryName, ResCityName, ResAddress };
		enum ViewColumns { ViewId, ViewEntity, ViewTimestampIn, ViewTimestampOut, ViewCountry, ViewCity, ViewAddress };

		virtual void load() override;

	protected:
		virtual void initFilters() override;
		virtual void initModel() override;
	};


	class AttendanceEntryFormModel : public RecordFormModel<AttendanceEntry>
	{
	public:
		static const Wt::WFormModel::Field entityField;
		static const Wt::WFormModel::Field dateInField;
		static const Wt::WFormModel::Field timeInField;
		static const Wt::WFormModel::Field dateOutField;
		static const Wt::WFormModel::Field timeOutField;
		static const Wt::WFormModel::Field locationField;

		AttendanceEntryFormModel(AttendanceEntryView *view, Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr = Wt::Dbo::ptr<AttendanceEntry>());
		virtual Wt::WWidget *createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		void updateTimestampOutValidator(bool update);
		AttendanceEntryView *_view = nullptr;

		friend class AttendanceEntryView;
	};

	class AttendanceEntryView : public RecordFormView
	{
	public:
		AttendanceEntryView(Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr = Wt::Dbo::ptr<AttendanceEntry>());
		virtual void initView() override;

		Wt::Dbo::ptr<AttendanceEntry> attendanceEntryPtr() const { return _model->recordPtr(); }
		AttendanceEntryFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return attendanceEntryPtr() ? AttendanceEntry::viewInternalPath(attendanceEntryPtr().id()) : ""; }
		virtual RecordFormView *createFormView() override { return new AttendanceEntryView(); }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		AttendanceEntryFormModel *_model = nullptr;
		Wt::Dbo::ptr<AttendanceEntry> _tempPtr;
	};
}

#endif