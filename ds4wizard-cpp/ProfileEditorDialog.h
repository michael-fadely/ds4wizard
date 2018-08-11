#pragma once

#include <QWidget>
#include "ui_ProfileEditorDialog.h"

class ProfileEditorDialog : public QWidget
{
	Q_OBJECT

public:
	ProfileEditorDialog(QWidget* parent = Q_NULLPTR);
	~ProfileEditorDialog();

private:
	Ui::ProfileEditorDialog ui;
};
