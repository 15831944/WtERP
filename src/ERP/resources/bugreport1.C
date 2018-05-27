#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDateEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WDialog.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WFormModel.h>

using namespace Wt;

class HelloApplication : public WApplication
{
public:
  HelloApplication(const WEnvironment& env);
};

const WFormModel::Field startDateField = "startDate";
const WFormModel::Field endDateField = "endDate";

HelloApplication::HelloApplication(const WEnvironment& env)
  : WApplication(env)
{
	setTitle("HELP ME :'(");

	//FORM MODEL//
	WFormModel *model = new WFormModel(this);
	model->addField(startDateField);
	model->addField(endDateField);

	//FORM VIEW//
	WTemplateFormView *view = new WTemplateFormView("${startDate}<br />${endDate}");

	//Start date
	auto startDateEdit = new Wt::WDateEdit();
	auto startDateValidator = new Wt::WDateValidator();
	startDateValidator->setMandatory(true);
	startDateValidator->setBottom(Wt::WDate(boost::gregorian::day_clock::local_day()));
	model->setValidator(startDateField, startDateValidator);
	view->setFormWidget(startDateField, startDateEdit);

	//End date
	auto endDateEdit = new Wt::WDateEdit();
	auto endDateValidator = new Wt::WDateValidator();
	model->setValidator(endDateField, endDateValidator);

	startDateEdit->changed().connect(std::bind([startDateEdit, endDateValidator]() {
		endDateValidator->setBottom(startDateEdit->date().addDays(1));
	}));
	view->setFormWidget(endDateField, endDateEdit);

	//Update view
	view->updateView(model);

	//DIALOG//
	WDialog *dialog = new Wt::WDialog(this);
	dialog->contents()->addWidget(view);

	Wt::WPushButton *submitBtn = new Wt::WPushButton("Continue", dialog->contents());
	submitBtn->clicked().connect(std::bind([=]() {
		view->updateModel(model);
		bool valid = model->validate();
		view->updateView(model);

		if(valid)
			dialog->accept();
	}));

	dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
		if(code == Wt::WDialog::Accepted)
		{
			dialog->contents()->removeWidget(view);
			root()->addWidget(view);
		}
		delete dialog;
	}, std::placeholders::_1));

	dialog->show();
}

WApplication *createApplication(const WEnvironment& env)
{
  return new HelloApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

