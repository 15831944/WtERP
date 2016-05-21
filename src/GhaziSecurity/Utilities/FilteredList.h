#ifndef GS_LISTFILTERMODEL_UTILITY_H
#define GS_LISTFILTERMODEL_UTILITY_H

#include "Application/WApplication.h"
#include "Utilities/ReloadableWidget.h"

#include <Wt/WObject>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WLineEdit>

#define IdColumnWidth 80
#define DateColumnWidth 120
#define DateTimeColumnWidth 150

namespace GS
{
	class AbstractFilteredList;

	//FITLER MODEL
	class AbstractFilterWidgetModel : public Wt::WObject
	{
	public:
		virtual void updateModel();
		virtual std::string value() = 0;
		virtual std::string sqlCondition() = 0;
		virtual Wt::WWidget *createWidget();
		virtual std::vector<std::string> boundValues() { return std::vector<std::string>(1, value()); }
		Wt::WString filterTitle() const { return _filterTitle; };
		Wt::WCheckBox *checkbox() const { return _cb; }
		bool enabled() const { return _enabled; }

	protected:
		AbstractFilterWidgetModel(const Wt::WString &filterTitle, const std::string &columnName, Wt::WObject *parent = nullptr)
			: Wt::WObject(parent), _columnName(columnName), _filterTitle(filterTitle)
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
		WLineEditFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: AbstractFilterWidgetModel(filterTitle, columnName, parent), _function(f)
		{ }
		//virtual void updateView() override { _edit->setValueText(_value); }
		virtual void updateModel() override;
		virtual std::string value() override { return _value.toUTF8(); }
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WLineEdit *edit() const { return _edit; }
		virtual Wt::WWidget *createWidget() override;

	protected:
		Wt::WLineEdit *_edit = nullptr;
		Wt::WString _value;
		FuncType _function;
	};

	class WComboBoxFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(Wt::WComboBox*)> FuncType;
		WComboBoxFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: AbstractFilterWidgetModel(filterTitle, columnName, parent), _function(f)
		{ }
		//virtual void updateView() override { _edit->setCurrentIndex(_value); }
		virtual void updateModel() override;
		virtual std::string value() override;
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WComboBox *edit() const { return _edit; }
		virtual Wt::WWidget *createWidget() override;

	protected:
		Wt::WComboBox *_edit = nullptr;
		int _value;
		FuncType _function;
	};

	class BitmaskFilterModel : public WComboBoxFilterModel
	{
	public:
		BitmaskFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: WComboBoxFilterModel(filterTitle, columnName, f, parent)
		{ }
		virtual std::string sqlCondition() override { return _columnName + " & ?"; }
		virtual void updateModel() override;
	};

	class NameFilterModel : public WLineEditFilterModel
	{
	public:
		NameFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: WLineEditFilterModel(filterTitle, columnName, f, parent)
		{ }
		virtual std::string sqlCondition() override;
		virtual std::vector<std::string> boundValues() override;
		virtual Wt::WWidget *createWidget() override;
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

		RangeEdit(Wt::WContainerWidget *parent = nullptr);
		Wt::WComboBox *operatorCombo() const { return _operatorCombo; }

	protected:
		Wt::WComboBox *_operatorCombo = nullptr;
	};

	class RangeFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(RangeEdit*)> FuncType;
		RangeFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: AbstractFilterWidgetModel(filterTitle, columnName, parent), _function(f)
		{ }
		virtual void updateModel() override;
		virtual std::string value() override { return _value.toUTF8(); }
		RangeEdit::Operators sqlOperator() const { return _operator; }
		virtual std::string sqlCondition() override;
		RangeEdit *edit() const { return _edit; }
		virtual Wt::WWidget *createWidget() override;

	protected:
		RangeEdit *_edit = nullptr;
		Wt::WString _value;
		RangeEdit::Operators _operator = RangeEdit::Equal;
		FuncType _function;
	};

	class FiltersTemplate : public Wt::WTemplate
	{
	public:
		typedef std::vector<AbstractFilterWidgetModel*> FilterModelVector;

		FiltersTemplate(AbstractFilteredList *filteredList, Wt::WContainerWidget *parent = nullptr);
		Wt::WContainerWidget *filterWidgetsContainer() const { return _filterWidgetsContainer; }

		void addFilterModel(AbstractFilterWidgetModel *model);
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
		Wt::WAbstractItemModel *model() const { return _model; }
		Wt::WAbstractProxyModel *proxyModel() const { return _proxyModel; }

	protected:
		AbstractFilteredList();

		virtual void initFilters() { }
		virtual void initModel() = 0;
		void addColumn(int viewIndex, int column, const Wt::WString &header, int width);

		typedef std::map<int, int> ViewIndexToColumnMap;
		ViewIndexToColumnMap _viewIndexToColumnMap;

		Wt::WTableView *_tableView = nullptr;
		FiltersTemplate *_filtersTemplate = nullptr;
		Wt::WAbstractItemModel *_model = nullptr;
		Wt::WAbstractProxyModel *_proxyModel = nullptr;
	};

	template<typename T>
	class QueryModelFilteredList : public AbstractFilteredList
	{
	public:
		typedef typename T ResultType;
		typedef Wt::Dbo::QueryModel<ResultType> QueryModelType;

		QueryModelType *queryModel() const { return dynamic_cast<QueryModelType*>(_model); }
		virtual void reload() override;

	protected:
		virtual void applyFilter(const std::string &sqlCondition) override;

		Wt::Dbo::Query<ResultType> _baseQuery;
	};

	template<class FilteredList, typename IdType = long long>
	class ListSelectionDialog : public Wt::WDialog
	{
	public:
		ListSelectionDialog(const Wt::WString &title, Wt::WObject *parent = nullptr);
		FilteredList *listWidget() const { return _listWidget; }
		Wt::Signal<IdType> &selected() { return _selected; }

	protected:
		void handleSelected();

		FilteredList *_listWidget;
		Wt::Signal<IdType> _selected;
	};

	template<class FilteredList, typename IdType>
	ListSelectionDialog<FilteredList, IdType>::ListSelectionDialog(const Wt::WString &title, Wt::WObject *parent)
		: Wt::WDialog(title, parent), _selected(this)
	{
		setClosable(true);
		resize(900, Wt::WLength(95, Wt::WLength::Percentage));
		setTransient(true);
		rejectWhenEscapePressed(true);
		setDeleteWhenHidden(true);
		contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);

		_listWidget = new FilteredList();
		_listWidget->enableFilters();
		_listWidget->tableView()->setSelectionMode(Wt::SingleSelection);
		_listWidget->tableView()->setSelectionBehavior(Wt::SelectRows);
		_listWidget->tableView()->selectionChanged().connect(boost::bind(&ListSelectionDialog::handleSelected, this));
		contents()->addWidget(_listWidget);
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

		auto queryModel = dynamic_cast<Wt::Dbo::QueryModel<FilteredList::ResultType>*>(_listWidget->model());
		if(index.model() != queryModel)
		{
			Wt::log("error") << "ListSelectionDialog::handleSelected(): index.model() != queryModel";
			return;
		}

		_selected.emit(boost::get<FilteredList::ResId>(queryModel->resultRow(index.row())));
		accept();
	}

	//TEMPLATE CLASS DEFINITIONS
	template<typename T>
	void QueryModelFilteredList<T>::reload()
	{
		try
		{
			if(loaded())
				queryModel()->reload();
			else
				load();
		}
		catch(Wt::Dbo::Exception &e)
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
		Wt::Dbo::Query<ResultType> query(_baseQuery);

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
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "QueryModelFilteredList::applyFilter(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}
}

#endif