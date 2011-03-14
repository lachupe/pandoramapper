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


#include <QByteArray>
#include <QMutex>


#include "SpellsDialog.h"
         
/* --------------------------- SpellsDialog  ------------------------------ */

SpellsDialog::SpellsDialog (QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    connect(pushButton_add, SIGNAL(clicked()), this, SLOT(add_clicked()) );
    connect(pushButton_save, SIGNAL(clicked()), this, SLOT(save_clicked()) );
    connect(pushButton_remove, SIGNAL(clicked()), this, SLOT(remove_clicked()) );
    connect(pushButton_edit, SIGNAL(clicked()), this, SLOT(edit_clicked()) );
}

void SpellsDialog::run()
{
    unsigned int    i;

    if (isVisible())
        return;

    spells.clear();
    disableFrame();
    editing_index = 0;
    
    for (i = 0; i < conf->spells.size(); i++) 
        spells.push_back( conf->spells[i] ); 
    
    redraw();
    show();
    raise();
    activateWindow();
}

void SpellsDialog::load_item_data(int index)
{
    lineEdit_name->setText(spells[index].name);
    lineEdit_up_mes->setText(spells[index].up_mes);
    lineEdit_refresh_mes->setText(spells[index].refresh_mes);
    lineEdit_down_mes->setText(spells[index].down_mes);
    checkBox_addon->setChecked( spells[index].addon );
}

void SpellsDialog::edit_clicked()
{
    editing_index = listWidget->currentRow();
    enableFrame();

    load_item_data(editing_index);
}

void SpellsDialog::redraw()
{
    unsigned int i;
    QString s;
    QString tmp, tmp2;
    
    listWidget->clear();
    for (i = 0; i < spells.size(); i++) {
/*        tmp = Events[patterns[i].event.type].data;
        tmp2 = patterns[i].pattern;
        s = QString("%1 %2 %3")
            .arg( patterns[i].is_regexp ? "REGEXP" : "WILDCARD", -10)
            .arg( tmp, -15)
            .arg( tmp2, -90 );
*/
        listWidget->addItem(spells[i].name);   
    }
    listWidget->setCurrentRow(0);
    repaint();
}


void SpellsDialog::save_clicked()
{
    TSpell p;
    
    disableFrame();
    
    if (editing_index == -1) {
        editing_index = spells.size();
        p.up = false;
        spells.push_back(p);
    }
    

    spells[editing_index].name = lineEdit_name->text().toAscii();
    spells[editing_index].up_mes = lineEdit_up_mes->text().toAscii();
    spells[editing_index].refresh_mes = lineEdit_refresh_mes->text().toAscii();
    spells[editing_index].down_mes = lineEdit_down_mes->text().toAscii();
    spells[editing_index].addon =  checkBox_addon->isChecked();
    
    redraw();
}

void SpellsDialog::add_clicked()
{
    if (frame->isEnabled()) {
        editing_index = -1;    
        disableFrame();
        save_clicked();
    } else {
        enableFrame();
        editing_index = -1;
        
        lineEdit_name->setText("");
        lineEdit_up_mes->setText("");
        lineEdit_refresh_mes->setText("");
        lineEdit_down_mes->setText("");
        checkBox_addon->setChecked( false );

    }
}

void SpellsDialog::remove_clicked()
{
    vector<TSpell>::iterator p;
    int i;
    
    i = 0;
    for (p=spells.begin(); p != spells.end(); ++p) {
        if (i == listWidget->currentRow()) {
            p = spells.erase(p);
            break;
        }
        i++;
    }
    redraw();
}


void SpellsDialog::accept()
{
    unsigned int i;
    
    conf->spells.clear();
    for (i = 0; i < spells.size(); i++) {
        conf->addSpell(spells[i]);
    }
    spells.clear();        
    done(Accepted);
}


void SpellsDialog::enableFrame()
{
    frame->setEnabled(true);
    pushButton_remove->setEnabled(false);
}

void SpellsDialog::disableFrame()
{    
    frame->setEnabled(false);
    pushButton_remove->setEnabled(true);
}


void SpellsDialog::on_listWidget_itemSelectionChanged()
{
	load_item_data( listWidget->currentRow() );
}
