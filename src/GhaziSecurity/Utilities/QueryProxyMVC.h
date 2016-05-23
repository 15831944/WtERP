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
			Wt::WAbstractItemModel *srcModel = sourceModel();
			Wt::Dbo::QueryModel<Result> *qryModel = nullptr;

			std::vector<Wt::WAbstractProxyModel*> srcProxyModels;
			do
			{
				if(!srcModel)
				{
					Wt::log("error") << "QueryProxyModel::indexOf() called without a source QueryModel";
					return -1;
				}

				qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(srcModel);
				if(!qryModel)
				{
					if(Wt::WAbstractProxyModel *srcProxyModel = dynamic_cast<Wt::WAbstractProxyModel*>(srcModel))
					{
						srcProxyModels.push_back(srcProxyModel);
						srcModel = srcProxyModel->sourceModel();
					}
					else
						srcModel = nullptr;
				}
			}
			while(!qryModel);

			int sourceRow = qryModel->indexOf(result);
			if(sourceRow == -1)
				return -1;

			Wt::WModelIndex idx = qryModel->index(sourceRow, 0);
			if(!idx.isValid())
				return -1;

			for(auto itr = srcProxyModels.rbegin(); itr != srcProxyModels.rend(); ++itr)
			{
				idx = (*itr)->mapFromSource(idx);
				if(!idx.isValid())
					return -1;
			}

			idx = mapFromSource(idx);
			if(!idx.isValid())
				return -1;

			return idx.row();
		}

		const Result *resultRow(int proxyRow) const
		{
			if(proxyRow == -1)
				return nullptr;

			Wt::WModelIndex idx = mapToSource(index(proxyRow, 0));
			if(!idx.isValid())
				return nullptr;

			Wt::WAbstractItemModel *srcModel = sourceModel();
			Wt::Dbo::QueryModel<Result> *qryModel = nullptr;

			do
			{
				if(!srcModel)
				{
					Wt::log("error") << "QueryProxyModel::resultRow() called without a source QueryModel";
					return nullptr;
				}

				qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(srcModel);
				if(!qryModel)
				{
					if(Wt::WAbstractProxyModel *srcProxyModel = dynamic_cast<Wt::WAbstractProxyModel*>(srcModel))
					{
						idx = srcProxyModel->mapToSource(idx);
						if(!idx.isValid())
							return nullptr;

						srcModel = srcProxyModel->sourceModel();
					}
					else
						srcModel = nullptr;
				}
			}
			while(!qryModel);

			if(!idx.isValid())
				return nullptr;

			return &qryModel->resultRow(idx.row());
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
				Wt::log("warning") << "ProxyModelCBValidator::validator(): null proxyModel";
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