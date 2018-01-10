#include "Utilities/FilteredList.h"

#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WTableView.h>
#include <Wt/WPushButton.h>
#include <Wt/WIntValidator.h>
#include <Wt/WDoubleValidator.h>
#include <Wt/WAbstractProxyModel.h>

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

	unique_ptr<Wt::WCheckBox> AbstractFilterWidgetModel::createCheckbox()
	{
		auto cb = make_unique<Wt::WCheckBox>();
		_cb = cb.get();
		return cb;
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
			_value = Wt::any_cast<int>(_edit->model()->index(_edit->currentIndex(), 0).data(Wt::ItemDataRole::User));
		else
			_value = 0;

		if(_value == 0)
			_enabled = false;
	}

	unique_ptr<Wt::WWidget> WLineEditFilterModel::createWidget()
	{
		auto edit = make_unique<Wt::WLineEdit>();
		_edit = edit.get();
		if(_function) _function(_edit);
		return edit;
	}

	unique_ptr<Wt::WWidget> WComboBoxFilterModel::createWidget()
	{
		auto edit = make_unique<Wt::WComboBox>();
		_edit = edit.get();
		if(_function) _function(_edit);
		return edit;
	}

	unique_ptr<Wt::WWidget> NameFilterModel::createWidget()
	{
		auto edit = WLineEditFilterModel::createWidget();
		_edit->setMaxLength(70);
		return edit;
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

	FiltersTemplate::FiltersTemplate(AbstractFilteredList *filteredList)
		: Wt::WTemplate(tr("GS.FiltersTemplate")), _filteredList(filteredList)
	{
		_filtersComboBox = bindNew<Wt::WComboBox>("filters-combo");
		_filtersComboBox->addItem(tr("SelectFilter"));

		auto addFilter = bindNew<Wt::WPushButton>("add-filter", tr("Add"));
		addFilter->clicked().connect(this, &FiltersTemplate::handleAddFilter);

		auto applyFilters = bindNew<Wt::WPushButton>("apply-filter", tr("ApplyFilters"));
		applyFilters->clicked().connect(this, &FiltersTemplate::handleApplyFilters);

		_filterWidgetsContainer = bindNew<Wt::WContainerWidget>("filters-container");
	}

	AbstractFilteredList::AbstractFilteredList()
	{
		setTemplateText(tr("GS.FilteredListView"));

		_tableView = bindNew<Wt::WTableView>("table-view");
		_tableView->setSelectable(true);
		_tableView->setHeaderHeight(40);
		_tableView->setRowHeight(30);
		_tableView->setMaximumSize(Wt::WLength::Auto, 600);
		_tableView->setAlternatingRowColors(true);
	}

	void AbstractFilteredList::load()
	{
		if(!loaded())
		{
			initModel();

			if(_proxyModel)
				_tableView->setModel(_proxyModel);
			else
				_tableView->setModel(_model);

			resetColumnWidths();

			if(_proxyModel)
			{
				int diff = _proxyModel->columnCount() - _model->columnCount();
				if(diff > 0)
				{
					//_tableView->setColumnWidth(_proxyModel->columnCount() - 1, 40);
					_tableView->setColumnAlignment(_proxyModel->columnCount() - 1, Wt::AlignmentFlag::Center);
				}

				while(diff > 0)
				{
					_tableView->setSortingEnabled(_model->columnCount() + diff - 1, false);
					--diff;
				}
			}

			if(_filtersTemplate)
				initFilters();
		}

		ReloadOnVisibleWidget::load();
	}

	void AbstractFilteredList::enableFilters()
	{
		if(conditionValue("filters-enabled"))
			return;

		setCondition("filters-enabled", true);
		_filtersTemplate = bindNew<FiltersTemplate>("filters", this);
		if(loaded())
			initFilters();
	}

	void AbstractFilteredList::resetColumnWidths()
	{
		auto model = _tableView->model();
		for(int i = 0; i < model->columnCount(); ++i)
		{
			Wt::any width = model->headerData(i, Wt::Orientation::Horizontal, Wt::ItemDataRole::Width);
			if(!width.empty())
				_tableView->setColumnWidth(i, Wt::WLength(Wt::any_cast<int>(width)));
		}
	}

	void AbstractFilteredList::addColumn(int viewIndex, int column, const Wt::WString &header, int width)
	{
		_model->setHeaderData(column, Wt::Orientation::Horizontal, header);
		_model->setHeaderData(column, Wt::Orientation::Horizontal, viewIndex, Wt::ItemDataRole::ViewIndex);
		_model->setHeaderData(column, Wt::Orientation::Horizontal, width, Wt::ItemDataRole::Width);

		_viewIndexToColumnMap[viewIndex] = column;
	}

	int AbstractFilteredList::viewIndexToColumn(int viewIndex) const
	{
		auto fitr = _viewIndexToColumnMap.find(viewIndex);
		return fitr == _viewIndexToColumnMap.end() ? -1 : fitr->second;
	}

	void FiltersTemplate::addFilterModel(shared_ptr<AbstractFilterWidgetModel> model)
	{
		if(!model)
			return;

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

		 Wt::WTemplate *filterTemplate = _filterWidgetsContainer->addNew<Wt::WTemplate>();
		if(auto rangeEdit = dynamic_cast<RangeEdit*>(newWidget.get()))
		{
			filterTemplate->setTemplateText(tr("GS.ListRangeFilterView"));
			filterTemplate->bindWidget("operatorCombo", rangeEdit->createOperatorCombo());
		}
		else
			filterTemplate->setTemplateText(tr("GS.ListFilterView"));

		filterTemplate->bindWidget("checkbox", model->createCheckbox());
		filterTemplate->bindString("title", model->filterTitle());
		filterTemplate->bindWidget("widget", std::move(newWidget));
		model->checkbox()->setChecked(true);
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
		edit->setValidator(make_shared<Wt::WIntValidator>());
		edit->setMaxLength(20);
	}

	RangeEdit::RangeEdit()
	{
		setTextSize(30);
		setValidator(make_shared<Wt::WDoubleValidator>());
	}

	unique_ptr<Wt::WComboBox> RangeEdit::createOperatorCombo()
	{
		auto combo = make_unique<Wt::WComboBox>();
		_operatorCombo = combo.get();
		_operatorCombo->insertItem(Equal, "=");
		_operatorCombo->insertItem(LessThan, "<");
		_operatorCombo->insertItem(LessThanEqual, "<=");
		_operatorCombo->insertItem(GreaterThan, ">");
		_operatorCombo->insertItem(GreaterThanEqual, ">=");

		return combo;
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

	unique_ptr<Wt::WWidget> RangeFilterModel::createWidget()
	{
		auto edit = make_unique<RangeEdit>();
		_edit = edit.get();
		if(_function) _function(_edit);
		return edit;
	}

}