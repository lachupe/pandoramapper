#ifndef CGROUPSETTINGSDIALOG_H_
#define CGROUPSETTINGSDIALOG_H_

#include <QDialog>
#include <QColor>
#include "ui_groupmanagersettings.h"



class CGroupSettingsDialog : public QDialog, public Ui::GroupManagerSettings {
Q_OBJECT
	QColor color;
public:
	CGroupSettingsDialog(QWidget *parent = 0);
	virtual ~CGroupSettingsDialog();
    void run();
public slots:
    void accept();
    void selectColor();

};

#endif /*CGROUPSETTINGSDIALOG_H_*/
