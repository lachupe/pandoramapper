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


#include <QMessageBox>

#include "utils.h"
#include "defines.h"
#include "CConfigurator.h"

#include "Map/CRoomManager.h"

#include "Engine/CStacksManager.h"

#include "Gui/CMovementDialog.h"
#include "Gui/CLogDialog.h"

CLogDialog::CLogDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);                        
    setWindowTitle(tr("Pandora's Log File"));
}

void CLogDialog::run()
{
    QUrl url;

    url.fromLocalFile( *logFileName );

    QFile file(*logFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        textBrowser->append(line);
    }

    textBrowser->setReadOnly(true);
}

void CLogDialog::accept()
{

    done(Accepted);
}
