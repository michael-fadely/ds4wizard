#include "pch.h"
#include "ProfileEditorDialog.h"
#include "DeviceProfileModel.h"

// TODO: maybe https://doc.qt.io/qt-5/qdatawidgetmapper.html#details

ProfileEditorDialog::ProfileEditorDialog(DeviceProfileModel* profileModel_, QWidget* parent)
	: QDialog(parent),
	  profileModel(profileModel_)
{
	ui.setupUi(this);

	auto model = profileModel->makeModifierModel(ui.comboBox_ModifierList);

	ui.comboBox_ModifierList->setModel(model);
}

ProfileEditorDialog::~ProfileEditorDialog() = default;
