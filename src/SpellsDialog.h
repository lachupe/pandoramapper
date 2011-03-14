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

#ifndef SPELLSDIALOG_H 
#define SPELLSDITDIALOG_H 

#include <QDialog>
#include "ui_spellsdialog.h"
#include "CConfigurator.h"

class SpellsDialog : public QDialog, public Ui::SpellsDialog {
Q_OBJECT

    int editing_index;
    
    vector<TSpell> spells;
    void enableFrame();
    void disableFrame();
    void load_item_data(int index);

public:
    SpellsDialog(QWidget *parent = 0);
    void run();
    void redraw();
public slots:
    void edit_clicked();
    void save_clicked();
    void add_clicked();
    void remove_clicked();
    virtual void accept();
private slots:
    void on_listWidget_itemSelectionChanged();
};


#endif
