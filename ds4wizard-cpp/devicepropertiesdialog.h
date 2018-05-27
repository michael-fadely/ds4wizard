#pragma once

#include <QDialog>
#include "ui_devicepropertiesdialog.h"

class DevicePropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	DevicePropertiesDialog(QWidget *parent = Q_NULLPTR);
	~DevicePropertiesDialog();

private:
	Ui::DevicePropertiesDialog ui;
};
