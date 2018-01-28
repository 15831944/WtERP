#ifndef ERP_ATTENDANCEDEVICE_MVC_H
#define ERP_ATTENDANCEDEVICE_MVC_H

#include "Common.h"
#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include "Utilities/RecordFormView.h"
#include "Widgets/FindRecordEdit.h"

#include <Wt/Dbo/QueryModel.h>
#include <Wt/WBatchEditProxyModel.h>

namespace ERP
{
	class AttendanceDeviceView;
	class AttendanceEntryView;

	//ATTENDANCE DEVICE
	class AttendanceDeviceListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AttendanceDeviceListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AttendanceDeviceList : public QueryModelFilteredList<tuple<long long, std::string, std::string, std::string, std::string>>
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

		AttendanceDeviceFormModel(AttendanceDeviceView *view, Dbo::ptr<AttendanceDevice> attendanceDevicePtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		AttendanceDeviceView *_view = nullptr;
	};

	class AttendanceDeviceView : public RecordFormView
	{
	public:
		AttendanceDeviceView(Dbo::ptr<AttendanceDevice> attendanceDevicePtr = nullptr);
		virtual void initView() override;

		Dbo::ptr<AttendanceDevice> attendanceDevicePtr() const { return _model->recordPtr(); }
		AttendanceDeviceFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return attendanceDevicePtr() ? AttendanceDevice::viewInternalPath(attendanceDevicePtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AttendanceDeviceView>(); }

	protected:
		AttendanceDeviceFormModel *_model;
		Dbo::ptr<AttendanceDevice> _tempPtr;
	};

	//ATTENDANCE ENTRY
	class AttendanceEntryListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AttendanceEntryListProxyModel(shared_ptr<Wt::WAbstractItemModel> model);
		virtual Wt::any data(const Wt::WModelIndex &idx, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::any headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AttendanceEntryList : public QueryModelFilteredList<tuple<long long, std::string, Wt::WDateTime, Wt::WDateTime, std::string, std::string, std::string>>
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

		AttendanceEntryFormModel(AttendanceEntryView *view, Dbo::ptr<AttendanceEntry> attendanceEntryPtr = nullptr);
		virtual unique_ptr<Wt::WWidget> createFormWidget(Field field) override;
		virtual bool saveChanges() override;

	protected:
		void updateTimestampOutValidator(bool update);
		AttendanceEntryView *_view = nullptr;

		friend class AttendanceEntryView;
	};

	class AttendanceEntryView : public RecordFormView
	{
	public:
		AttendanceEntryView(Dbo::ptr<AttendanceEntry> attendanceEntryPtr = nullptr);
		virtual void initView() override;

		Dbo::ptr<AttendanceEntry> attendanceEntryPtr() const { return _model->recordPtr(); }
		AttendanceEntryFormModel *model() const { return _model; }

		virtual Wt::WString viewName() const override;
		virtual std::string viewInternalPath() const override { return attendanceEntryPtr() ? AttendanceEntry::viewInternalPath(attendanceEntryPtr().id()) : ""; }
		virtual unique_ptr<RecordFormView> createFormView() override { return make_unique<AttendanceEntryView>(); }

	protected:
		virtual void updateView(Wt::WFormModel *model) override;
		virtual void updateModel(Wt::WFormModel *model) override;

		AttendanceEntryFormModel *_model = nullptr;
		Dbo::ptr<AttendanceEntry> _tempPtr;
	};
}

#endif