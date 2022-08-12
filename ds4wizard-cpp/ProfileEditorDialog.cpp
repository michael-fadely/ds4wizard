#include "pch.h"
#include "ProfileEditorDialog.h"
#include "DeviceProfileModel.h"

// TODO: maybe https://doc.qt.io/qt-5/qdatawidgetmapper.html#details

ProfileEditorDialog::ProfileEditorDialog(DeviceProfileModel* profileModel_, QWidget* parent)
	: QDialog(parent),
	  profileModel(profileModel_)
{
	ui.setupUi(this);

	ui.lineEdit_ProfileName->setText(QString::fromStdString(profileModel_->profile.name));
	ui.comboBox_ModifierList->setModel(profileModel->makeModifierModel(ui.comboBox_ModifierList));
}

ProfileEditorDialog::~ProfileEditorDialog() = default;
