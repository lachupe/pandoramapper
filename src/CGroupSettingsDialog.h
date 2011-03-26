/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
