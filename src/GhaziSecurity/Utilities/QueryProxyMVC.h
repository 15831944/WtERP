#ifndef GS_SELECTADDPROXYMODEL_H
#define GS_SELECTADDPROXYMODEL_H

#include <Wt/WBatchEditProxyModel>
#include <Wt/WSortFilterProxyModel>
#include <Wt/Dbo/QueryModel>
#include <Wt/WComboBox>
#include <Wt/WValidator>

namespace GS
{
	
	template<typename R>
	class QueryProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		typedef typename R Result;

		QueryProxyModel(Wt::WObject *parent = nullptr) : Wt::WBatchEditProxyModel(parent) { }
		int indexOf(const Result &result) const
		{
			auto thisModel = this;
			auto srcModel = sourceModel();
			auto qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(srcModel);
			Wt::WAbstractProxyModel *proxyModel = nullptr;
			if(!qryModel)
			{
				proxyModel = dynamic_cast<Wt::WAbstractProxyModel*>(srcModel);
				if(proxyModel)
					qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(proxyModel->sourceModel());
			}

			if(!qryModel)
			{
				Wt::log("error") << "QueryProxyModel::indexOf() called without a source QueryModel";
				return -1;
			}

			int sourceRow = qryModel->indexOf(result);
			if(sourceRow == -1)
				return sourceRow;

			Wt::WModelIndex idx;
			if(proxyModel)
			{
				//When there is a WSortFilterProxyModel between proxyModel and queryModel
				//srcModel should be WSortFilterProxyModel
				idx = proxyModel->mapFromSource(qryModel->index(sourceRow, 0));
				idx = thisModel->mapFromSource(idx);
			}
			else
				idx = thisModel->mapFromSource(qryModel->index(sourceRow, 0));

			return idx.row();
		}
		const Result *resultRow(int proxyRow) const
		{
			if(proxyRow == -1)
				return nullptr;

			auto thisModel = this;
			auto qryModel = queryModel();
			if(!qryModel)
			{
				Wt::log("error") << "QueryProxyModel::resultRow() called without a source QueryModel";
				return nullptr;
			}

			auto sourceIndex = thisModel->mapToSource(thisModel->index(proxyRow, 0));
			if(!sourceIndex.isValid())
				return nullptr;

			return &qryModel->resultRow(sourceIndex.row());
		}

		Wt::Dbo::QueryModel<Result> *queryModel() const
		{
			auto qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(sourceModel());
			if(!qryModel)
			{
				auto proxyModel = dynamic_cast<Wt::WAbstractProxyModel*>(sourceModel());
				if(proxyModel)
					qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(proxyModel->sourceModel());
			}
			return qryModel;
		}
	};

	class AbstractQueryProxyModelCB : public Wt::WComboBox
	{
	protected:
		AbstractQueryProxyModelCB(Wt::WAbstractItemModel *model, Wt::WContainerWidget *parent = nullptr)
			: Wt::WComboBox(parent)
		{
			setModel(model);
			setCurrentIndex(0);
		}

	public:
		virtual void setViewValue(const boost::any &v) = 0;
		virtual boost::any modelValue() = 0;
	};

	template<class QueryProxyModel>
	class QueryProxyModelCB : public AbstractQueryProxyModelCB
	{
	public:
		QueryProxyModelCB(QueryProxyModel *model, Wt::WContainerWidget *parent = nullptr) : AbstractQueryProxyModelCB(model, parent) { }
		virtual void setViewValue(const boost::any &v) override
		{
			if(v.empty())
			{
				setCurrentIndex(0);
				return;
			}

			const auto &result = boost::any_cast<QueryProxyModel::Result>(v);
			auto proxyModel = dynamic_cast<QueryProxyModel*>(model());
			setCurrentIndex(std::max(0, proxyModel->indexOf(result)));
		}
		virtual boost::any modelValue() override
		{
			auto proxyModel = dynamic_cast<QueryProxyModel*>(model());
			const QueryProxyModel::Result *res = proxyModel->resultRow(currentIndex());
			if(res)
				return *res;
			else
				return QueryProxyModel::Result();
		}
	};

	class BaseComboBoxValidator : public Wt::WValidator
	{
	public:
		BaseComboBoxValidator(Wt::WObject *parent) : Wt::WValidator(parent) { }
		void setErrorString(const Wt::WString &str) { _errorStr = str; }

	protected:
		Wt::WString _errorStr;
	};

	template<class QueryProxyModel>
	class QueryProxyModelCBValidator : public BaseComboBoxValidator
	{
	public:
		//typedef typename ProxyModel ProxyModel;
		QueryProxyModelCBValidator(QueryProxyModelCB<QueryProxyModel> *cb)
			: BaseComboBoxValidator((Wt::WObject*)cb), _cb(cb)
		{ }
		virtual Result validate(const Wt::WString &input) const override
		{
			if(_cb->currentIndex() == -1)
				return Result(Invalid, _errorStr);

			auto proxyModel = dynamic_cast<QueryProxyModel*>(_cb->model());
			auto result = proxyModel->resultRow(_cb->currentIndex());
			if(!result)
				return Result(Invalid, _errorStr);

			return Result(Valid);
		}

	protected:
		QueryProxyModelCB<QueryProxyModel> *_cb = nullptr;
	};

	class ProxyModelCBValidator : public BaseComboBoxValidator
	{
	public:
		//typedef typename ProxyModel ProxyModel;
		ProxyModelCBValidator(Wt::WComboBox *cb)
			: BaseComboBoxValidator((Wt::WObject*)cb), _cb(cb)
		{ }
		virtual Result validate(const Wt::WString &input) const override
		{
			if(_cb->currentIndex() == -1)
				return Result(Invalid, _errorStr);

			auto proxyModel = dynamic_cast<Wt::WAbstractProxyModel*>(_cb->model());
			if(!proxyModel)
			{
				Wt::log("warn") << "ProxyModelCBValidator::validator(): null proxyModel";
				return Result(Invalid, Wt::WString::tr("Error"));
			}

			auto sourceIndex = proxyModel->mapToSource(proxyModel->index(_cb->currentIndex(), 0));
			if(!sourceIndex.isValid())
				return Result(Invalid, _errorStr);

			return Result(Valid);
		}

	protected:
		Wt::WComboBox *_cb = nullptr;
	};

}

#endif