#ifndef GS_MYTEMPLATEFORMVIEW_H
#define GS_MYTEMPLATEFORMVIEW_H

#include "Utilities/QueryProxyModel.h"
#include <Wt/WTemplateFormView>
#include <Wt/WComboBox>

namespace GS
{
	class AbstractProxyModelCB : public Wt::WComboBox
	{
	protected:
		AbstractProxyModelCB(Wt::WAbstractItemModel *model, Wt::WContainerWidget *parent = nullptr)
			: Wt::WComboBox(parent)
		{
			setModel(model);
			setCurrentIndex(0);
		}

	public:
		virtual void setViewValue(const boost::any &v) = 0;
		virtual boost::any modelValue() = 0;
	};

	template<class ProxyModel>
	class ProxyModelComboBox : public AbstractProxyModelCB
	{
	public:
		ProxyModelComboBox(ProxyModel *model, Wt::WContainerWidget *parent = nullptr) : AbstractProxyModelCB(model, parent) { }
		virtual void setViewValue(const boost::any &v) override
		{
			if(v.empty())
			{
				setCurrentIndex(0);
				return;
			}

			const auto &result = boost::any_cast<ProxyModel::Result>(v);
			auto proxyModel = dynamic_cast<ProxyModel*>(model());
			setCurrentIndex(std::max(0, proxyModel->indexOf(result)));
		}
		virtual boost::any modelValue() override
		{
			auto proxyModel = dynamic_cast<ProxyModel*>(model());
			const ProxyModel::Result *res = proxyModel->resultRow(currentIndex());
			if(res)
				return *res;
			else
				return ProxyModel::Result();
		}
	};

	class MyTemplateFormView : public Wt::WTemplateFormView
	{
	public:
		MyTemplateFormView(Wt::WContainerWidget *parent = nullptr) : Wt::WTemplateFormView(parent) { addFunction("fwId", &Wt::WTemplate::Functions::fwId); }
		MyTemplateFormView(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : Wt::WTemplateFormView(text, parent) { addFunction("fwId", &Wt::WTemplate::Functions::fwId); }

		virtual void updateView(Wt::WFormModel *model) override;
		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;

		virtual Wt::WString viewName() const { return tr("Unknown"); }
		virtual std::string viewInternalPath() const { return ""; }
		virtual MyTemplateFormView *createFormView() { return nullptr; }
		Wt::Signal<void> &submitted() { return _submitted; }

	protected:
		Wt::Signal<void> _submitted;
	};

	class BaseProxyModelComboBoxValidator : public Wt::WValidator
	{
	public:
		BaseProxyModelComboBoxValidator(Wt::WObject *parent) : Wt::WValidator(parent) { }
		void setErrorString(const Wt::WString &str) { _errorStr = str; }

	protected:
		Wt::WString _errorStr;
	};

	template<class ProxyModel>
	class ProxyModelCBValidator : public BaseProxyModelComboBoxValidator
	{
	public:
		//typedef typename ProxyModel ProxyModel;
		ProxyModelCBValidator(ProxyModelComboBox<ProxyModel> *cb)
			: BaseProxyModelComboBoxValidator((Wt::WObject*)cb), _cb(cb)
		{ }
		virtual Result validate(const Wt::WString &input) const override
		{
			if(_cb->currentIndex() == -1)
				return Result(Invalid, _errorStr);

			auto proxyModel = dynamic_cast<ProxyModel*>(_cb->model());
			auto result = proxyModel->resultRow(_cb->currentIndex());
			if(!result)
				return Result(Invalid, _errorStr);

			return Result(Valid);
		}

	protected:
		ProxyModelComboBox<ProxyModel> *_cb = nullptr;
	};
}

#endif