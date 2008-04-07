#ifndef CONFIGWIDGET_H 
#define CONFIGWIDGET_H

#include <QDialog>
#include "ui_configedit.h"


class ConfigWidget : public QDialog, public Ui::ConfigDialog {
Q_OBJECT
public:
    ConfigWidget (QWidget *parent = 0);
    
    void run();
public slots:
    virtual void accept();
    void autorefreshUpdated(bool);
    virtual void selectNoteColor();
};


#endif
