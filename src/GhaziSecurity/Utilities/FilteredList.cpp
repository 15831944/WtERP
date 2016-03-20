#include "Utilities/FilteredList.h"

#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>
#include <Wt/WDoubleValidator>

#include <boost/tokenizer.hpp>

namespace GS
{

	void AbstractFilterWidgetModel::updateModel()
	{
		if(_cb)
			_enabled = _cb->isChecked();
		else
			_enabled = false;
	}

	Wt::WWidget *AbstractFilterWidgetModel::createWidget()
	{
		return _cb = new Wt::WCheckBox();
	}

	void WLineEditFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = _edit->valueText();
		else
			_value = "";

		if(_value.empty())
			_enabled = false;
	}

	void WComboBoxFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = _edit->currentIndex();
		else
			_value = -1;

		if(_value == -1)
			_enabled = false;
	}

	std::string WComboBoxFilterModel::value()
	{
		return boost::lexical_cast<std::string>(_value);
	}

	void BitmaskFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = boost::any_cast<int>(_edit->model()->index(_edit->currentIndex(), 0).data(Wt::UserRole));
		else
			_value = 0;

		if(_value == 0)
			_enabled = false;
	}

	Wt::WWidget *WLineEditFilterModel::createWidget()
	{
		if(_edit)
			return nullptr;

		_edit = new Wt::WLineEdit();
		if(_function) _function(_edit);

		AbstractFilterWidgetModel::createWidget();
		return _edit;
	}

	Wt::WWidget *WComboBoxFilterModel::createWidget()
	{
		if(_edit)
			return nullptr;

		_edit = new Wt::WComboBox();
		if(_function) _function(_edit);

		AbstractFilterWidgetModel::createWidget();
		return _edit;
	}

	Wt::WWidget *NameFilterModel::createWidget()
	{
		if(!WLineEditFilterModel::createWidget())
			return nullptr;

		_edit->setMaxLength(70);
		return _edit;
	}

	std::string NameFilterModel::sqlCondition()
	{
		std::string result;
		std::string v = value();
		boost::tokenizer<> tokens(v);
		for(const auto &namePart : tokens)
			result += _columnName + " LIKE ? OR ";

		return result.substr(0, result.size() - 4);
	}

	std::vector<std::string> NameFilterModel::boundValues()
	{
		std::string v = value();
		boost::tokenizer<> tokens(v);
		auto vec = std::vector<std::string>(tokens.begin(), tokens.end());
		for(auto &s : vec)
			s = "%" + s + "%";
		return vec;
	}

	FiltersTemplate::FiltersTemplate(AbstractFilteredList *filteredList, Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate(tr("GS.FiltersTemplate"), parent), _filteredList(filteredList)
	{
		addFunction("tr", &Wt::WTemplate::Functions::tr);

		_filtersComboBox = new Wt::WComboBox();
		_filtersComboBox->addItem(tr("SelectFilter"));

		auto addFilter = new Wt::WPushButton(tr("Add"));
		addFilter->clicked().connect(this, &FiltersTemplate::handleAddFilter);

		auto applyFilters = new Wt::WPushButton(tr("ApplyFilters"));
		applyFilters->clicked().connect(this, &FiltersTemplate::handleApplyFilters);

		_filterWidgetsContainer = new Wt::WContainerWidget();

		bindWidget("filters-combo", _filtersComboBox);
		bindWidget("add-filter", addFilter);
		bindWidget("apply-filter", applyFilters);
		bindWidget("filters-container", _filterWidgetsContainer);
	}

	AbstractFilteredList::AbstractFilteredList(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate(tr("GS.FilteredListView"), parent)
	{
		addFunction("tr", Wt::WTemplate::Functions::tr);

		_tableView = new Wt::WTableView();
		_tableView->setSelectable(true);
		_tableView->setHeaderHeight(40);
		_tableView->setRowHeight(30);
		_tableView->setMaximumSize(Wt::WLength::Auto, 600);
		_tableView->setAlternatingRowColors(true);
		bindWidget("table-view", _tableView);
	}

	void AbstractFilteredList::enableFilters()
	{
		if(conditionValue("filters-enabled"))
			return;

		setCondition("filters-enabled", true);
		bindWidget("filters", _filtersTemplate = new FiltersTemplate(this));
		initFilters();
	}

	void AbstractFilteredList::init()
	{
		if(_model || _proxyModel)
			return;

		initModel();
		_tableView->setModel(_proxyModel ? _proxyModel : _model);
		resetColumnWidths();

		int diff = _proxyModel->columnCount() - _model->columnCount();
		if(diff > 0)
		{
			//_tableView->setColumnWidth(_proxyModel->columnCount() - 1, 40);
			_tableView->setColumnAlignment(_proxyModel->columnCount() - 1, Wt::AlignCenter);
		}

		while(diff > 0)
		{
			_tableView->setSortingEnabled(_model->columnCount() + diff - 1, false);
			--diff;
		}
	}

	void AbstractFilteredList::resetColumnWidths()
	{
		auto model = _tableView->model();
		for(int i = 0; i < model->columnCount(); ++i)
		{
			boost::any width = model->headerData(i, Wt::Horizontal, Wt::WidthRole);
			if(!width.empty())
				_tableView->setColumnWidth(i, Wt::WLength(boost::any_cast<int>(width)));
		}
	}

	void AbstractFilteredList::addColumn(int viewIndex, int column, const Wt::WString &header, int width)
	{
		_model->setHeaderData(column, Wt::Horizontal, header);
		_model->setHeaderData(column, Wt::Horizontal, viewIndex, Wt::ViewIndexRole);
		_model->setHeaderData(column, Wt::Horizontal, width, Wt::WidthRole);

		_viewIndexToColumnMap[viewIndex] = column;
	}

	int AbstractFilteredList::viewIndexToColumn(int viewIndex) const
	{
		auto fitr = _viewIndexToColumnMap.find(viewIndex);
		return fitr == _viewIndexToColumnMap.end() ? -1 : fitr->second;
	}

	void FiltersTemplate::addFilterModel(AbstractFilterWidgetModel *model)
	{
		if(!model)
			return;

		Wt::WObject::addChild(model);
		_filtersComboBox->addItem(model->filterTitle());
		_modelVector.push_back(model);
	}

	void FiltersTemplate::handleAddFilter()
	{
		addFilter(_filtersComboBox->currentIndex());
	}

	void FiltersTemplate::addFilter(int filtersComboIndex)
	{
		if(filtersComboIndex < 1 || filtersComboIndex > _modelVector.size())
			return;

		auto model = _modelVector[filtersComboIndex - 1];
		auto newWidget = model->createWidget();
		if(!newWidget)
			return;

		Wt::WTemplate *filterTemplate = new Wt::WTemplate();
		if(auto rangeEdit = dynamic_cast<RangeEdit*>(newWidget))
		{
			filterTemplate->setTemplateText(tr("GS.ListRangeFilterView"));
			filterTemplate->bindWidget("operatorCombo", rangeEdit->operatorCombo());
		}
		else
			filterTemplate->setTemplateText(tr("GS.ListFilterView"));

		model->checkbox()->setChecked(true);
		filterTemplate->bindWidget("checkbox", model->checkbox());
		filterTemplate->bindString("title", model->filterTitle());
		filterTemplate->bindWidget("widget", newWidget);
		_filterWidgetsContainer->addWidget(filterTemplate);
	}

	void FiltersTemplate::handleApplyFilters()
	{
		std::string sqlCondition;
		for(auto model : _modelVector)
		{
			model->updateModel();
			if(!model->enabled())
				continue;

			std::string thisCondition = model->sqlCondition();
			if(thisCondition.empty())
				continue;

			thisCondition = "(" + thisCondition + ")";
			sqlCondition += thisCondition + " AND ";
		}
		sqlCondition = sqlCondition.substr(0, sqlCondition.size() - 5);
		_filteredList->applyFilter(sqlCondition);
	}

	void FiltersTemplate::initIdEdit(Wt::WLineEdit *edit)
	{
		edit->setValidator(new Wt::WIntValidator());
		edit->setMaxLength(20);
	}

	RangeEdit::RangeEdit(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WLineEdit(parent)
	{
		setTextSize(30);

		auto lineEditValidator = new Wt::WDoubleValidator(this);
		setValidator(lineEditValidator);

		_operatorCombo = new Wt::WComboBox();
		_operatorCombo->insertItem(Equal, "=");
		_operatorCombo->insertItem(LessThan, "<");
		_operatorCombo->insertItem(LessThanEqual, "<=");
		_operatorCombo->insertItem(GreaterThan, ">");
		_operatorCombo->insertItem(GreaterThanEqual, ">=");
	}

	void RangeFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
		{
			_value = _edit->valueText();
			_operator = RangeEdit::Operators(_edit->operatorCombo()->currentIndex());
		}
		else
		{
			_value = "";
			_operator = RangeEdit::Equal;
		}

		if(_value.empty())
			_enabled = false;
	}

	std::string RangeFilterModel::sqlCondition()
	{
		switch(_operator)
		{
		default:
		case RangeEdit::Equal: return _columnName + " = ?";
		case RangeEdit::LessThan: return _columnName + " < ?";
		case RangeEdit::LessThanEqual: return _columnName + " <= ?";
		case RangeEdit::GreaterThan: return _columnName + " > ?";
		case RangeEdit::GreaterThanEqual: return _columnName + " >= ?";
		}
	}

	Wt::WWidget *RangeFilterModel::createWidget()
	{
		if(_edit)
			return nullptr;

		_edit = new RangeEdit();
		if(_function) _function(_edit);

		AbstractFilterWidgetModel::createWidget();
		return _edit;
	}

}