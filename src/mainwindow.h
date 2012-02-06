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

#ifndef MAINWINDOW_H 
#define MAINWINDOW_H 

#include <QMainWindow>
#include <QDockWidget>
#include <QTextBrowser>

#include "renderer.h"
#include "CActionManager.h"
#include "CSelectionManager.h"


/* 
 * Use forward declarations instead of including relevant header files to make
 * compiling faster.
 */
class QLabel;
class ConfigWidget;
class RoomEditDialog;
class SpellsDialog;
class CMovementDialog;
class CLogDialog;
class FindDialog;

#include "CGroup.h"
#include "CGroupSettingsDialog.h"

void toggle_renderer_reaction();
void notify_analyzer();


class DockWidget : public QDockWidget
{
  Q_OBJECT
public:
  DockWidget ( const QString & title, QWidget * parent = 0, Qt::WFlags flags = 0 );

  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;

};

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    
    CActionManager *actionManager;
    CMouseState     mouseState;
    
    QLabel        *locationLabel; 
    QLabel        *formulaLabel; 
    QLabel        *modLabel; 

    QMenu       *fileMenu;
    QMenu       *mappingMenu;
    QMenu       *actionsMenu;
    QMenu       *toolsMenu;
    QMenu       *optionsMenu;
    QMenu       *logMenu;
    QMenu       *helpMenu;
    QMenu		*groupMenu;

    // port of a log dockable from mmapper
    QTextBrowser   *logWindow;
    DockWidget *m_dockDialogLog;
    DockWidget *m_dockDialogGroup;

 
    QPoint mousePosInRenderer( QPoint pos );
    bool checkMouseSelection( QMouseEvent *e ); 

    enum ToolMode {
        SelectMode,
        MoveMode,
        DeleteMode
    };

    CGroup *groupManager;

public:
    CMainWindow(QWidget *parent = 0);

    RendererWidget *renderer;
    ConfigWidget      *generalSettingsDialog;
    RoomEditDialog    *edit_dialog;
    SpellsDialog      *spells_dialog;
    CMovementDialog   *movementDialog;
    CLogDialog        *logdialog;
    FindDialog        *findDialog;
    CGroupSettingsDialog *groupDialog;
    
    QAction       *hide_status_action;
    QAction       *hide_menu_action;


    void update_status_bar();
    void editRoomDialog(unsigned int id);
    void setToolMode(ToolMode mode);
    QRect getGroupManagerRect() { return groupManager->geometry(); }
    CGroup *getGroupManager() { return groupManager; }
    void setShowGroupManager(bool b) { 
    	if (b == true) 
    		groupManager->show();
    	else 
    		groupManager->hide(); }

private:
    ToolMode toolMode;

    void createContextMenu( QMouseEvent *e );

private slots:
  bool event(QEvent *event);
  void mousePressEvent( QMouseEvent *);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *);
  void wheelEvent(QWheelEvent *);
  void keyPressEvent( QKeyEvent * );
  void keyReleaseEvent( QKeyEvent * );

public slots:
  void hide_menu();
  void hide_status();
  void hide();                              /* hide all extra widgets */

  void moveRoomDialog();
  void setMapMoveMode();
  void setDeleteMode();
  void setSelectMode();
  void addDockLogEntry(const QString& module, const QString& message);



protected slots:
    void closeEvent(QCloseEvent *event);

signals:
  void newLocationLabel(const QString &label);
  void newModLabel(const QString &label);
  void mapMoveValueChanged(bool);
};


extern class CMainWindow *renderer_window;



#endif
