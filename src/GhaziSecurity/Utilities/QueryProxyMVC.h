#ifndef GS_SELECTADDPROXYMODEL_H
#define GS_SELECTADDPROXYMODEL_H

#include "Common.h"
#include <Wt/WBatchEditProxyModel.h>
#include <Wt/WSortFilterProxyModel.h>
#include <Wt/Dbo/QueryModel.h>
#include <Wt/WComboBox.h>
#include <Wt/WValidator.h>

namespace GS
{
	
	template<typename R>
	class QueryProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		typedef R Result;

		int indexOf(const Result &result) const
		{
			shared_ptr<Wt::WAbstractItemModel> srcModel = sourceModel();
			shared_ptr<Dbo::QueryModel<Result>> qryModel;

			std::vector<shared_ptr<Wt::WAbstractProxyModel>> srcProxyModels;
			do
			{
				if(!srcModel)
				{
					Wt::log("error") << "QueryProxyModel::indexOf() called without a source QueryModel";
					return -1;
				}

				qryModel = dynamic_pointer_cast<Dbo::QueryModel<Result>>(srcModel);
				if(!qryModel)
				{
					if(shared_ptr<Wt::WAbstractProxyModel> srcProxyModel = dynamic_pointer_cast<Wt::WAbstractProxyModel>(srcModel))
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

			shared_ptr<Wt::WAbstractItemModel> srcModel = sourceModel();
			shared_ptr<Dbo::QueryModel<Result>> qryModel;

			do
			{
				if(!srcModel)
				{
					Wt::log("error") << "QueryProxyModel::resultRow() called without a source QueryModel";
					return nullptr;
				}

				qryModel = dynamic_pointer_cast<Dbo::QueryModel<Result>>(srcModel);
				if(!qryModel)
				{
					if(shared_ptr<Wt::WAbstractProxyModel> srcProxyModel = dynamic_pointer_cast<Wt::WAbstractProxyModel>(srcModel))
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
		AbstractQueryProxyModelCB(shared_ptr<Wt::WAbstractItemModel> model)
		{
			setModel(model);
			setCurrentIndex(0);
		}

	public:
		virtual void setViewValue(const Wt::any &v) = 0;
		virtual Wt::any modelValue() = 0;
	};

	template<class QueryProxyModel>
	class QueryProxyModelCB : public AbstractQueryProxyModelCB
	{
	public:
		QueryProxyModelCB(shared_ptr<QueryProxyModel> model) : AbstractQueryProxyModelCB(model) { }
		virtual void setViewValue(const Wt::any &v) override
		{
			if(v.empty())
			{
				setCurrentIndex(0);
				return;
			}

			const auto &result = Wt::any_cast<typename QueryProxyModel::Result>(v);
			shared_ptr<QueryProxyModel> proxyModel = static_pointer_cast<QueryProxyModel>(model());
			setCurrentIndex(std::max(0, proxyModel->indexOf(result)));
		}
		virtual Wt::any modelValue() override
		{
			shared_ptr<QueryProxyModel> proxyModel = static_pointer_cast<QueryProxyModel>(model());
			const typename QueryProxyModel::Result *res = proxyModel->resultRow(currentIndex());
			if(res)
				return *res;
			else
				return typename QueryProxyModel::Result();
		}
	};

	class BaseComboBoxValidator : public Wt::WValidator
	{
	public:
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
			: _cb(cb)
		{ }
		virtual Result validate(const Wt::WString &input) const override
		{
			if(_cb->currentIndex() == -1)
				return Result(Wt::ValidationState::Invalid, _errorStr);

			auto proxyModel = static_pointer_cast<QueryProxyModel>(_cb->model());
			auto result = proxyModel->resultRow(_cb->currentIndex());
			if(!result)
				return Result(Wt::ValidationState::Invalid, _errorStr);

			return Result(Wt::ValidationState::Valid);
		}

	protected:
		QueryProxyModelCB<QueryProxyModel> *_cb = nullptr;
	};

	class ProxyModelCBValidator : public BaseComboBoxValidator
	{
	public:
		//typedef typename ProxyModel ProxyModel;
		ProxyModelCBValidator(Wt::WComboBox *cb)
			:_cb(cb)
		{ }
		virtual Result validate(const Wt::WString &input) const override
		{
			if(_cb->currentIndex() == -1)
				return Result(Wt::ValidationState::Invalid, _errorStr);

			auto proxyModel = dynamic_pointer_cast<Wt::WAbstractProxyModel>(_cb->model());
			if(!proxyModel)
			{
				Wt::log("warning") << "ProxyModelCBValidator::validator(): null proxyModel";
				return Result(Wt::ValidationState::Invalid, tr("Error"));
			}

			auto sourceIndex = proxyModel->mapToSource(proxyModel->index(_cb->currentIndex(), 0));
			if(!sourceIndex.isValid())
				return Result(Wt::ValidationState::Invalid, _errorStr);

			return Result(Wt::ValidationState::Valid);
		}

	protected:
		Wt::WComboBox *_cb = nullptr;
	};

}

#endif