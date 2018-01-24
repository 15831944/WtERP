#ifndef ERP_LISTFILTERMODEL_UTILITY_H
#define ERP_LISTFILTERMODEL_UTILITY_H

#include "Common.h"
#include "Application/WApplication.h"
#include "Utilities/ReloadableWidget.h"

#include <Wt/WObject.h>
#include <Wt/WString.h>
#include <Wt/WTemplate.h>
#include <Wt/WLineEdit.h>
#include <Wt/WModelIndex.h>

#define IdColumnWidth 80
#define DateColumnWidth 120
#define DateTimeColumnWidth 150
#define EmailColumnWidth 150

namespace ERP
{
	class AbstractFilteredList;

	//FITLER MODEL
	class AbstractFilterWidgetModel : public Wt::WObject
	{
	public:
		virtual void updateModel();
		virtual std::string value() = 0;
		virtual std::string sqlCondition() = 0;
		virtual std::vector<std::string> boundValues() { return std::vector<std::string>(1, value()); }

		unique_ptr<Wt::WCheckBox> createCheckbox();
		virtual unique_ptr<Wt::WWidget> createWidget() = 0;

		Wt::WString filterTitle() const { return _filterTitle; };
		Wt::WCheckBox *checkbox() const { return _cb; }

		bool enabled() const { return _enabled; }

	protected:
		AbstractFilterWidgetModel(Wt::WString filterTitle, std::string columnName)
			: _columnName(move(columnName)), _filterTitle(move(filterTitle))
		{ }
		Wt::WString _filterTitle;
		std::string _columnName;
		Wt::WCheckBox *_cb = nullptr;
		bool _enabled = false;
	};

	class WLineEditFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(Wt::WLineEdit*)> FuncType;
		WLineEditFilterModel(const Wt::WString &filterTitle, const std::string &columnName, FuncType f = FuncType())
			: AbstractFilterWidgetModel(filterTitle, columnName), _function(move(f))
		{ }
		//virtual void updateView() override { _edit->setValueText(_value); }
		virtual void updateModel() override;
		virtual std::string value() override { return _value.toUTF8(); }
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WLineEdit *edit() const { return _edit; }
		virtual unique_ptr<Wt::WWidget> createWidget() override;

	protected:
		Wt::WLineEdit *_edit = nullptr;
		Wt::WString _value;
		FuncType _function;
	};

	class WComboBoxFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(Wt::WComboBox*)> FuncType;
		WComboBoxFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType())
			: AbstractFilterWidgetModel(filterTitle, columnName), _function(f)
		{ }
		//virtual void updateView() override { _edit->setCurrentIndex(_value); }
		virtual void updateModel() override;
		virtual std::string value() override;
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WComboBox *edit() const { return _edit; }
		virtual unique_ptr<Wt::WWidget> createWidget() override;

	protected:
		Wt::WComboBox *_edit = nullptr;
		int _value;
		FuncType _function;
	};

	class BitmaskFilterModel : public WComboBoxFilterModel
	{
	public:
		BitmaskFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType())
			: WComboBoxFilterModel(filterTitle, columnName, f)
		{ }
		virtual std::string sqlCondition() override { return _columnName + " & ?"; }
		virtual void updateModel() override;
	};

	class NameFilterModel : public WLineEditFilterModel
	{
	public:
		NameFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType())
			: WLineEditFilterModel(filterTitle, columnName, f)
		{ }
		virtual std::string sqlCondition() override;
		virtual std::vector<std::string> boundValues() override;
		virtual unique_ptr<Wt::WWidget> createWidget() override;
	};

	class RangeEdit : public Wt::WLineEdit
	{
	public:
		enum Operators
		{
			Equal,
			LessThan,
			LessThanEqual,
			GreaterThan,
			GreaterThanEqual
		};

		RangeEdit();
		unique_ptr<Wt::WComboBox> createOperatorCombo();
		Wt::WComboBox *operatorCombo() const { return _operatorCombo; }

	protected:
		Wt::WComboBox *_operatorCombo = nullptr;
	};

	class RangeFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(RangeEdit*)> FuncType;
		RangeFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType())
			: AbstractFilterWidgetModel(filterTitle, columnName), _function(f)
		{ }
		virtual void updateModel() override;
		virtual std::string value() override { return _value.toUTF8(); }
		RangeEdit::Operators sqlOperator() const { return _operator; }
		virtual std::string sqlCondition() override;
		RangeEdit *edit() const { return _edit; }
		virtual unique_ptr<Wt::WWidget> createWidget() override;

	protected:
		RangeEdit *_edit = nullptr;
		Wt::WString _value;
		RangeEdit::Operators _operator = RangeEdit::Equal;
		FuncType _function;
	};

	class FiltersTemplate : public Wt::WTemplate
	{
	public:
		typedef std::vector<shared_ptr<AbstractFilterWidgetModel>> FilterModelVector;

		FiltersTemplate(AbstractFilteredList *filteredList);
		Wt::WContainerWidget *filterWidgetsContainer() const { return _filterWidgetsContainer; }

		void addFilterModel(shared_ptr<AbstractFilterWidgetModel> model);
		void addFilter(int filtersComboIndex);

		static void initIdEdit(Wt::WLineEdit *edit);
		static void initEntityTypeEdit(Wt::WComboBox *edit);
		static void initRoleEdit(Wt::WComboBox *edit);
		
	protected:
		void handleAddFilter();
		void handleApplyFilters();

		Wt::WContainerWidget *_filterWidgetsContainer = nullptr;
		Wt::WComboBox *_filtersComboBox = nullptr;
		AbstractFilteredList *_filteredList = nullptr;
		FilterModelVector _modelVector;

	private:
		friend class AbstractFilteredList;
		template<typename T> friend class QueryModelFilteredList;
	};

	//LIST MODEL
	class AbstractFilteredList : public ReloadOnVisibleWidget<Wt::WTemplate>
	{
	public:
		virtual void load() override;
		virtual void applyFilter(const std::string &sqlCondition) = 0;
		void enableFilters();
		void resetColumnWidths();
		int viewIndexToColumn(int viewIndex) const;
		
		Wt::WTableView *tableView() const { return _tableView; }
		FiltersTemplate *filtersTemplate() const { return _filtersTemplate; }
		shared_ptr<Wt::WAbstractItemModel> model() const { return _model; }
		shared_ptr<Wt::WAbstractProxyModel> proxyModel() const { return _proxyModel; }

	protected:
		AbstractFilteredList();

		virtual void initFilters() { }
		virtual void initModel() = 0;
		void addColumn(int viewIndex, int column, const Wt::WString &header, int width);

		typedef std::map<int, int> ViewIndexToColumnMap;
		ViewIndexToColumnMap _viewIndexToColumnMap;

		Wt::WTableView *_tableView = nullptr;
		FiltersTemplate *_filtersTemplate = nullptr;
		shared_ptr<Wt::WAbstractItemModel> _model;
		shared_ptr<Wt::WAbstractProxyModel> _proxyModel;
	};

	template<typename T>
	class QueryModelFilteredList : public AbstractFilteredList
	{
	public:
		typedef T ResultType;
		typedef Dbo::QueryModel<ResultType> QueryModelType;

		shared_ptr<QueryModelType> queryModel() const { return dynamic_pointer_cast<QueryModelType>(_model); }
		virtual void reload() override;

	protected:
		virtual void applyFilter(const std::string &sqlCondition) override;
		virtual Dbo::Query<ResultType> generateQuery() const { return _baseQuery; }

		Dbo::Query<ResultType> _baseQuery;
	};

	template<class FilteredList, typename IdType = long long>
	class ListSelectionDialog : public Wt::WDialog
	{
	public:
		ListSelectionDialog(const Wt::WString &title);
		FilteredList *listWidget() const { return _listWidget; }
		Wt::Signal<IdType> &selected() { return _selected; }

	protected:
		void handleSelected();

		FilteredList *_listWidget;
		Wt::Signal<IdType> _selected;
	};

	template<typename FilteredList, typename IdType>
	ListSelectionDialog<FilteredList, IdType>::ListSelectionDialog(const Wt::WString &title)
		: Wt::WDialog(title)
	{
		resize(Wt::WLength(85,  Wt::LengthUnit::Percentage), Wt::WLength(95,  Wt::LengthUnit::Percentage));
		setClosable(true);
		setTransient(true);
		rejectWhenEscapePressed(true);
		contents()->setOverflow(Wt::Overflow::Auto);

		_listWidget = contents()->template addNew<FilteredList>();
		_listWidget->enableFilters();
		_listWidget->tableView()->setSelectionMode(Wt::SelectionMode::Single);
		_listWidget->tableView()->setSelectionBehavior(Wt::SelectionBehavior::Rows);
		_listWidget->tableView()->selectionChanged().connect(this, &ListSelectionDialog::handleSelected);
	}

	template<class FilteredList, typename IdType>
	void ListSelectionDialog<FilteredList, IdType>::handleSelected()
	{
		auto indexSet = _listWidget->tableView()->selectedIndexes();
		if(indexSet.empty())
			return;

		Wt::WModelIndex index = *indexSet.begin();
		if(!index.isValid())
			return;

		if(_listWidget->proxyModel())
		{
			index = _listWidget->proxyModel()->mapToSource(index);
			if(!index.isValid())
			{
				Wt::log("error") << "ListSelectionDialog::handleSelected(): could not make index to source";
				return;
			}
		}

		auto queryModel = static_pointer_cast<Dbo::QueryModel<typename FilteredList::ResultType>>(_listWidget->model());
		if(index.model() != queryModel.get())
		{
			Wt::log("error") << "ListSelectionDialog::handleSelected(): index.model() != queryModel";
			return;
		}

		_selected.emit(std::get<FilteredList::ResId>(queryModel->resultRow(index.row())));
		accept();
	}

	//TEMPLATE CLASS DEFINITIONS
	template<typename T>
	void QueryModelFilteredList<T>::reload()
	{
		try
		{
			if(loaded())
				queryModel()->setQuery(generateQuery(), true);
			else
				load();
		}
		catch(Dbo::Exception &e)
		{
			Wt::log("error") << "QueryModelFilteredList::reload(): Dbo error(" << e.code() << "): " << e.what();
			APP->showDbBackendError(e.code());
		}
	}

	template<typename T>
	void QueryModelFilteredList<T>::applyFilter(const std::string &sqlCondition)
	{
		if(!filtersTemplate())
			return;

		WApplication *app = APP;
		auto model = queryModel();
		Dbo::Query<ResultType> query(_baseQuery);

		if(!sqlCondition.empty())
		{
			query.where(sqlCondition);
			for(auto model : filtersTemplate()->_modelVector)
			{
				if(!model->enabled())
					continue;

				auto values = model->boundValues();
				for(const std::string &v : values)
					query.bind(v);
			}
		}

		try
		{
			model->setQuery(query, true);
		}
		catch(const Dbo::Exception &e)
		{
			Wt::log("error") << "QueryModelFilteredList::applyFilter(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}
}

#endif