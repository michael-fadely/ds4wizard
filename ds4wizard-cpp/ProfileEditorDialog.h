#pragma once

#include <QWidget>
#include "ui_ProfileEditorDialog.h"

class DeviceProfileModel;

class ProfileEditorDialog : public QDialog
{
	Q_OBJECT

	DeviceProfileModel* profileModel = nullptr;

public:
	ProfileEditorDialog(DeviceProfileModel* profileModel_, QWidget* parent = Q_NULLPTR);
	~ProfileEditorDialog();

private:
	Ui::ProfileEditorDialog ui;
};
