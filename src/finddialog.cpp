#include <QtGui>
#include "finddialog.h"
#include "CRoomManager.h"
#include "renderer.h"
#include "mainwindow.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    adjustResultTable();

    connect(lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableFindButton(const QString &)));
    connect(findButton, SIGNAL(clicked()),
            this, SLOT(findClicked()));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(resultTable, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(itemDoubleClicked(QTreeWidgetItem *)));
}

void FindDialog::on_lineEdit_textChanged()
{
    findButton->setEnabled(lineEdit->hasAcceptableInput());
}

void FindDialog::findClicked()
{
    QString text = lineEdit->text();
    Qt::CaseSensitivity cs =
            caseCheckBox->isChecked() ? Qt::CaseSensitive
                                      : Qt::CaseInsensitive;
    QTreeWidgetItem *item;
    QList<int> results;

    resultTable->clear();

    if (nameRadioButton->isChecked())
        results = Map.searchNames(text, cs);
    else if (descRadioButton->isChecked())
        results = Map.searchDescs(text, cs);
    else if (notesRadioButton->isChecked())
        results = Map.searchNotes(text, cs);
    else if (exitsRadioButton->isChecked())
        results = Map.searchExits(text, cs);

    for (int i = 0; i < results.size(); i++) {
        QString id = QString(tr("%1").arg( results.at(i) ));
        QString roomName = QString(Map.getName( results.at(i) ));

        item = new QTreeWidgetItem(resultTable);
        item->setText(0, id);
        item->setText(1, roomName);
    }

    roomsFoundLabel->setText(tr("%1 room(s) found").arg( results.size() ));
}

void FindDialog::itemDoubleClicked(QTreeWidgetItem *item)
{
    unsigned int id;

    if (item == NULL)
        return;

    id = item->text(0).toInt();

    Map.selections.exclusiveSelection(id);
    renderer_window->renderer->centerOnRoom(id);
}

void FindDialog::enableFindButton(const QString &text)
{
    findButton->setEnabled(!text.isEmpty());
}

void FindDialog::adjustResultTable()
{
    resultTable->setColumnCount(2);
    resultTable->setHeaderLabels(
            QStringList() << tr("Room ID") << tr("Room Name"));
//    resultTable->header()->setResizeMode(0, QHeaderView::Stretch);
    resultTable->header()->setResizeMode(1, QHeaderView::Stretch);
    resultTable->setRootIsDecorated(false);
    resultTable->setAlternatingRowColors(true);
}
