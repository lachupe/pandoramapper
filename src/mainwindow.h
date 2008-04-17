#ifndef MAINWINDOW_H 
#define MAINWINDOW_H 

#include <QMainWindow>

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

signals:
  void newLocationLabel(const QString &label);
  void newModLabel(const QString &label);
  void mapMoveValueChanged(bool);
};


extern class CMainWindow *renderer_window;



#endif
