#ifndef GS_SELECTADDPROXYMODEL_H
#define GS_SELECTADDPROXYMODEL_H

#include <Wt/WBatchEditProxyModel>
#include <Wt/WSortFilterProxyModel>
#include <Wt/Dbo/QueryModel>

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
			auto proxyModel = this;
			auto qryModel = queryModel();
			if(!qryModel)
			{
				Wt::log("error") << "QueryProxyModel::indexOf() called without a source QueryModel";
				return -1;
			}

			int sourceRow = qryModel->indexOf(result);
			if(sourceRow == -1)
				return sourceRow;

			return proxyModel->mapFromSource(qryModel->index(sourceRow, 0)).row();
		}
		const Result *resultRow(int proxyRow) const
		{
			if(proxyRow == -1)
				return nullptr;

			auto proxyModel = this;
			auto qryModel = queryModel();
			if(!qryModel)
			{
				Wt::log("error") << "QueryProxyModel::resultRow() called without a source QueryModel";
				return nullptr;
			}

			auto sourceIndex = proxyModel->mapToSource(proxyModel->index(proxyRow, 0));
			if(!sourceIndex.isValid())
				return nullptr;

			return &qryModel->resultRow(sourceIndex.row());
		}

		Wt::Dbo::QueryModel<Result> *queryModel() const
		{
			auto qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(sourceModel());
			if(!qryModel)
			{
				auto sortFilterModel = dynamic_cast<Wt::WSortFilterProxyModel*>(sourceModel());
				if(sortFilterModel)
					qryModel = dynamic_cast<Wt::Dbo::QueryModel<Result>*>(sortFilterModel->sourceModel());
			}
			return qryModel;
		}
	};

}

#endif