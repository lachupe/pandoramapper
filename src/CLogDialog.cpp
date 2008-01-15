
#include <QMessageBox>

#include "CConfigurator.h"
#include "CMovementDialog.h"
#include "CRoomManager.h"
#include "utils.h"
#include "CStacksManager.h"
#include "defines.h"
#include "CLogDialog.h"

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
