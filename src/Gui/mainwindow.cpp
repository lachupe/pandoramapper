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
#include <QtGui>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QtCore>
#include <QtWidgets>

#include "utils.h"


#include "Gui/mainwindow.h"
#include "Gui/CActionManager.h"
#include "Gui/roomeditattrdlg.h"
#include "Gui/ConfigWidget.h"
#include "Gui/SpellsDialog.h"
#include "Gui/CMovementDialog.h"
#include "Gui/CLogDialog.h"
#include "Gui/finddialog.h"
#include "Gui/CGroupSettingsDialog.h"


#include "Engine/CStacksManager.h"
#include "Engine/CEngine.h"
#include "Proxy/userfunc.h"
#include "Proxy/proxy.h"


class CMainWindow *renderer_window;


DockWidget::DockWidget ( const QString & title, QWidget * parent, Qt::WindowFlags flags )
    :QDockWidget(title, parent, flags)
{}

QSize DockWidget::minimumSizeHint() const
{
  return QSize(200, 0);
};

QSize DockWidget::sizeHint() const
{
  return QSize(500, 130);
};





void toggle_renderer_reaction()
{
    if (renderer_window->renderer->redraw == false) {
        renderer_window->renderer->redraw = true;
        proxy->startRendererCall();
    }
}

void notify_analyzer()
{
    proxy->startEngineCall();
}

/*  globals end */

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow( parent)
{
    spells_dialog = NULL;
    edit_dialog = NULL;
    generalSettingsDialog = NULL;
    movementDialog = NULL;
    logdialog = NULL;
    findDialog = NULL;
    groupDialog = NULL;

    userland_parser = new Userland();
    actionManager = new CActionManager(this);

    print_debug(DEBUG_INTERFACE, "in mainwindow constructor");

    setWindowTitle("Pandora");
    renderer =  new RendererWidget( this );
    setCentralWidget( renderer );

    if (!renderer->format().sampleBuffers()) {
    	print_debug(DEBUG_SYSTEM, "This system does not have sample buffer support.\r\n");
    	printf("This system does not have sample buffer support.\r\n");
     }

    setGeometry( conf->getWindowRect() );

    connect(proxy, SIGNAL(connectionEstablished()), actionManager, SLOT(enable_online_actions()), Qt::QueuedConnection );
    connect(proxy, SIGNAL(connectionLost()), actionManager, SLOT(disable_online_actions()), Qt::QueuedConnection );

    /* Enable mouse tracking to be able to show tooltips. */
    setMouseTracking(true);


    m_dockDialogLog = new DockWidget(tr("Log View"), this);
    m_dockDialogLog->setObjectName("DockWidgetLog");
    m_dockDialogLog->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    m_dockDialogLog->setFeatures(QDockWidget::AllDockWidgetFeatures);
    addDockWidget(Qt::BottomDockWidgetArea, m_dockDialogLog);

    logWindow = new QTextBrowser(m_dockDialogLog);
    logWindow->setObjectName("LogWindow");
    m_dockDialogLog->setWidget(logWindow);


    //setToolMode(SelectMode);
    setToolMode(conf->getStartupMode()?MoveMode:SelectMode);

    QMenuBar *bar = menuBar();

    /* now building a menu and adding actions to menu */
    fileMenu = bar->addMenu(tr("&File"));
    fileMenu->addAction(actionManager->newAct);
    fileMenu->addSeparator();
    fileMenu->addAction(actionManager->openAct);
    fileMenu->addAction(actionManager->reloadAct);
    fileMenu->addAction(actionManager->importAct);
    fileMenu->addSeparator();
    fileMenu->addAction(actionManager->saveAct);
    fileMenu->addAction(actionManager->saveAsAct);
    fileMenu->addAction(actionManager->exportAct);
    fileMenu->addSeparator();
    fileMenu->addAction(actionManager->quitAct);


    actionsMenu = menuBar()->addMenu(tr("&Room"));
    actionsMenu->addAction(actionManager->findAct);
    actionsMenu->addAction(actionManager->roomeditAct);
    actionsMenu->addAction(actionManager->moveRoomAct);
    actionsMenu->addAction(actionManager->deleteAct);
    actionsMenu->addAction(actionManager->deleteFullyAct);
    actionsMenu->addAction(actionManager->mergeAct);
    actionsMenu->addAction(actionManager->refreshAct);


    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(actionManager->selectToolAct);
    toolsMenu->addAction(actionManager->mapMoveToolAct);
    toolsMenu->addAction(actionManager->deleteToolAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actionManager->selectionTypeAct);




    mappingMenu = menuBar()->addMenu(tr("&Mapping"));
    mappingMenu->addAction(actionManager->mappingAct);
    mappingMenu->addAction(actionManager->automergeAct);
    mappingMenu->addAction(actionManager->angryLinkerAct);
    mappingMenu->addAction(actionManager->duallinkerAct);
    mappingMenu->addSeparator();
    mappingMenu->addAction(actionManager->publishAct);

    hide_status_action = new QAction(tr("Hide Status Bar"), this);
    hide_status_action->setShortcut(tr("F11"));
    hide_status_action->setStatusTip(tr("Hides the Statusbar"));
    hide_status_action->setCheckable(true);
    hide_status_action->setChecked(false);
    connect(hide_status_action, SIGNAL(triggered()), this, SLOT(hide_status()));

    hide_menu_action= new QAction(tr("Hide Menu Bar"), this);
    hide_menu_action->setShortcut(tr("F12"));
    hide_menu_action->setStatusTip(tr("Hides the Menubar"));
    hide_menu_action->setCheckable(true);
    hide_menu_action->setChecked(false);
    connect(hide_menu_action, SIGNAL(triggered()), this, SLOT(hide_menu()));


    optionsMenu = menuBar()->addMenu(tr("&Configuration"));
    optionsMenu->addAction(hide_status_action);
    optionsMenu->addAction(hide_menu_action);
    optionsMenu->addAction(actionManager->always_on_top_action);
    optionsMenu->addAction(actionManager->emulationAct);
    optionsMenu->addSeparator();
    optionsMenu->addAction(actionManager->setupGeneralAct);
    optionsMenu->addAction(actionManager->spellsAct);
    optionsMenu->addSeparator();
    optionsMenu->addAction(actionManager->saveConfigAct);
    //  optionsMenu->addAction(saveConfigAsAct);
    //optionsMenu->addAction(actionManager->loadConfigAct);



    logMenu = menuBar()->addMenu(tr("&Log") );
    logMenu->addAction(actionManager->showLogAct );
    logMenu->addAction(m_dockDialogLog->toggleViewAction());


    groupMenu = menuBar()->addMenu(tr("&Group") );
    groupMenu->addAction(actionManager->groupOffAct);
    groupMenu->addAction(actionManager->groupClientAct);
    groupMenu->addAction(actionManager->groupServerAct);
    groupMenu->addSeparator();
    groupMenu->addAction(actionManager->groupShowHideAct);
    groupMenu->addSeparator();
    groupMenu->addAction(actionManager->groupSettingsAct);
    groupMenu->addAction(actionManager->groupClearSpellsAct);


    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(actionManager->aboutAct);
    helpMenu->addAction(actionManager->aboutQtAct);




    /* status bar magicz */
    locationLabel = new QLabel("NO_SYNC");
    locationLabel->setAlignment(Qt::AlignHCenter);
    locationLabel->setMinimumSize(locationLabel->sizeHint());

    formulaLabel = new QLabel();

    modLabel = new QLabel(tr("     "));
    modLabel->setAlignment(Qt::AlignHCenter);
    modLabel->setMinimumSize(modLabel->sizeHint());
    modLabel->clear();

    statusBar()->addWidget(locationLabel);
    statusBar()->addWidget(formulaLabel, 1);
    statusBar()->addWidget(modLabel);

    connect(this, SIGNAL(newLocationLabel(const QString &)),
            locationLabel, SLOT(setText(const QString &)));
    connect(this, SIGNAL(newModLabel(const QString &)),
            modLabel, SLOT(setText(const QString &)));

    actionManager->disable_online_actions();
    connect(conf, SIGNAL(configurationChanged()),
            actionManager, SLOT(updateActionsSettings() ), Qt::QueuedConnection);

    groupManager = new CGroup(conf->getGroupManagerCharName(), this);
    connect(groupManager, SIGNAL(hides()), actionManager, SLOT( groupManagerHides() ),  Qt::QueuedConnection );
    connect(proxy, SIGNAL(sendGTell(QByteArray)), groupManager, SLOT( sendGTell(QByteArray) ),  Qt::QueuedConnection );
    connect(proxy, SIGNAL(sendScoreLine(QByteArray)), groupManager, SLOT( parseScoreInformation(QByteArray) ),  Qt::QueuedConnection );

    connect(proxy, SIGNAL(sendPromptLine(QByteArray)), groupManager, SLOT( parsePromptInformation(QByteArray) ),  Qt::QueuedConnection );
    connect(proxy, SIGNAL(sendPromptLine(QByteArray)), engine, SLOT( setPrompt(QByteArray) ),  Qt::QueuedConnection );

    connect(proxy, SIGNAL(sendSpellsUpdate()), groupManager, SLOT( updateSpellsInfo() ),  Qt::QueuedConnection );

    connect(proxy, SIGNAL(sendCharStateUpdate(int)), groupManager, SLOT( setCharState(int) ),  Qt::QueuedConnection );


    CGroupCommunicator *communicator = groupManager->getCommunicator();
    connect(communicator, SIGNAL(typeChanged(int)), actionManager, SLOT(groupManagerTypeChanged(int)), Qt::QueuedConnection  );

    connect(renderer, SIGNAL(updateCharPosition(unsigned int)), groupManager, SLOT( setCharPosition(unsigned int) ),  Qt::QueuedConnection );

    // bind all the posters to the dockable log
    connect(groupManager, SIGNAL(log( const QString&, const QString& )), this, SLOT(addDockLogEntry( const QString&, const QString& )));
    connect(proxy, SIGNAL(log( const QString&, const QString& )), this, SLOT(addDockLogEntry( const QString&, const QString& )));


    addDockLogEntry("General", "Started");
}


void CMainWindow::addDockLogEntry(const QString& module, const QString& message)
{
  logWindow->append("[" + module + "] " + message);
  logWindow->update();
}


void CMainWindow::moveRoomDialog()
{
    print_debug(DEBUG_INTERFACE, "move room dialog action called");

    // check if there is an objective for this operation
    if (Map.selections.size() == 0 && stacker.amount() != 1) {
        QMessageBox::critical(this, "Movement Dialog",
                             QString("You have to either get in sync or select some rooms!"));
        return;
    }

    // create the dialog if needed
    if (!movementDialog) {
        movementDialog = new CMovementDialog (this);
    }

    // launch the dialog
    movementDialog->show();
    movementDialog->raise();
    movementDialog->activateWindow();

}



void CMainWindow::hide_status()
{
  print_debug(DEBUG_INTERFACE, "hide/show status called");

  if (hide_status_action->isChecked() )
  {
    statusBar()->hide();
  } else {
    statusBar()->show();
  }
}

void CMainWindow::editRoomDialog()
{
    RoomEditAttrDlg m_roomEditDialog;


    m_roomEditDialog.setRoomSelection( Map.selections.getList(), &Map);
    m_roomEditDialog.exec();
}


void CMainWindow::hide()
{
  print_debug(DEBUG_INTERFACE, "hide/show all function");

  menuBar()->hide();
  statusBar()->hide();


  hide_menu_action->setChecked(true);
  hide_status_action->setChecked(true);
}

void CMainWindow::hide_menu()
{
  if (hide_menu_action->isChecked())
  {
    menuBar()->hide();
  } else {
    menuBar()->show();
  }
}


void CMainWindow::update_status_bar()
{
    char str[20];


    print_debug(DEBUG_INTERFACE, "Updating status bar\r\n");

    QString modLabel;
    QString firstPart;

    if (conf->isDatabaseModified() )
        firstPart = "Data: MOD ";
    else
        firstPart = "Data: --- ";

    modLabel = QString("Rooms Selected %1 ").arg( Map.selections.size() );

    emit newModLabel(modLabel + firstPart);

    stacker.getCurrent(str);
    emit newLocationLabel(str);
    print_debug(DEBUG_INTERFACE, "Done updating interface!\r\n");
}

/* Reimplement main even handler to catch tooltip events. */
bool CMainWindow::event(QEvent *event)
{
    unsigned int id;

    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (renderer->doSelect( mousePosInRenderer( helpEvent->pos() ), id ))
            QToolTip::showText(helpEvent->globalPos(), Map.getRoom(id)->toolTip());
        else
#if QT_VERSION >= 0x040200
            QToolTip::hideText();
#else
            QToolTip::showText(QPoint(), QString());
#endif
    }
    return QWidget::event(event);
}

void CMainWindow::keyPressEvent( QKeyEvent *k )
{

    print_debug(DEBUG_INTERFACE, "Processing events in keyPressEvent\r\n");
    switch ( k->key() ) {

        case Qt::Key_X :
        	renderer->setUserZ( renderer->getUserZ() + 1);
            toggle_renderer_reaction();
            break;

         case Qt::Key_Y:
          	renderer->setUserZ( renderer->getUserZ() - 1);
            toggle_renderer_reaction();
            break;

         case Qt::Key_Q:
           	renderer->setUserX( renderer->getUserX() - 1);
            toggle_renderer_reaction();
            break;

         case Qt::Key_W:
            	renderer->setUserX( renderer->getUserX() + 1);
            toggle_renderer_reaction();
            break;

         case Qt::Key_A:
           	renderer->setUserY( renderer->getUserY() + 1);
            toggle_renderer_reaction();
            break;

         case Qt::Key_S:
           	renderer->setUserY( renderer->getUserY() - 1);
            toggle_renderer_reaction();
            break;

        case Qt::Key_Up:
           	renderer->setAngleX( renderer->getAngleX() + 5);
            toggle_renderer_reaction();
            break;
        case Qt::Key_Down:
           	renderer->setAngleX( renderer->getAngleX() - 5);
            toggle_renderer_reaction();
            break;
        case Qt::Key_Left:
           	renderer->setAngleY( renderer->getAngleY() - 5);
            toggle_renderer_reaction();
            break;
        case Qt::Key_Right:
           	renderer->setAngleY( renderer->getAngleY() + 5);
            toggle_renderer_reaction();
            break;
        case Qt::Key_PageUp:
           	renderer->setAngleZ( renderer->getAngleZ() + 5);
            toggle_renderer_reaction();
            break;
        case Qt::Key_PageDown:
           	renderer->setAngleZ( renderer->getAngleZ() - 5);
            toggle_renderer_reaction();
            break;

        case Qt::Key_Plus:
            renderer->changeUserLayerShift( +1 );
            toggle_renderer_reaction();
            break;

        case Qt::Key_Minus:
            renderer->changeUserLayerShift( -1 );
            toggle_renderer_reaction();
            break;

        case Qt::Key_Escape:
        	renderer->resetViewSettings();
            toggle_renderer_reaction();
            break;

         case Qt::Key_F12:
             hide_menu_action->setChecked(!hide_menu_action->isChecked());
             hide_menu();
             break;
         case Qt::Key_F11:
        	 hide_status_action->setChecked(!hide_status_action->isChecked());
            hide_status();
            break;
         case Qt::Key_F10:
            //hide_roominfo();
            break;
         case Qt::Key_Alt:
                setToolMode(MoveMode);
            break;
    }
    update_status_bar();
    renderer->paintGL();
    print_debug(DEBUG_INTERFACE, "Done processing events at keyEventPress\r\n");
}

void CMainWindow::keyReleaseEvent( QKeyEvent *k )
{
    print_debug(DEBUG_INTERFACE, "Processing events in keyReleaseEvent\r\n");
    switch ( k->key() ) {
        case Qt::Key_Alt:
            setToolMode(SelectMode);
            break;
    }
}

void CMainWindow::mousePressEvent( QMouseEvent *e )
{
    mouseState.oldPos = e->pos();
    mouseState.origPos = e->pos();

    if (e->button() == Qt::LeftButton)
        mouseState.LeftButtonPressed = true;
    else if (e->button() == Qt::RightButton)
        mouseState.RightButtonPressed = true;

    switch (toolMode) {
    case MoveMode:
#if QT_VERSION >= 0x040200
        renderer->setCursor(Qt::ClosedHandCursor);
#endif
        break;
    case DeleteMode:
        if (e->button() == Qt::LeftButton) {
            if (checkMouseSelection(e) == true) {
                print_debug(DEBUG_INTERFACE, "Deleting room.");
                userland_parser->parse_user_input_line("mdelete");
            }
        }
        break;
    case SelectMode:
        if (e->button() == Qt::LeftButton) {
            if (checkMouseSelection(e) == true) {
                print_debug(DEBUG_INTERFACE, "Registered object selection with left mouse button.");
            }
        } else if (e->button() == Qt::RightButton) {
            if (checkMouseSelection(e) == true) {
                print_debug(DEBUG_INTERFACE, "Registered object selection with right mouse button.");

                createContextMenu(e);
            }
        }
        break;
    }
}

/*
 * FIXME: The dialogs that pop up from context menu get rooms from selection.
 * If more than one room was previously selected, strange results may happen.
 */
void CMainWindow::createContextMenu( QMouseEvent *e )
{
    unsigned int id;
    QAction *roomNameAct;
    QMenu menu(this);
    QString roomName;

//    if (Map.tryLockForRead() == false) {
//    	print_debug(DEBUG_GENERAL, "paintGL tried to block the eventQueue. Delayed.");
//    	QTimer::singleShot( 100, this, SLOT(createContextMenu(e)) );
//    	return;
//    } else
//    	Map.unlock();


    if (renderer->doSelect( mousePosInRenderer( e->pos() ), id )) {
        roomName = Map.getRoom(id)->getName();
    } else {
        roomName = tr("No room here");
    }

    /* Trim room name to 25 characters. */
    if (roomName.length() > 25)
        roomName = roomName.left(22) + "...";

    roomNameAct = new QAction(roomName, this);
    roomNameAct->setEnabled(false);
    menu.addAction(roomNameAct);
    menu.addSeparator();
    menu.addAction(actionManager->gotoAct);
    menu.addAction(actionManager->roomeditAct);
    menu.addAction(actionManager->moveRoomAct);
    menu.addAction(actionManager->deleteAct);
    menu.addAction(actionManager->deleteFullyAct);
    menu.addAction(actionManager->refreshAct);

    menu.addAction(actionManager->bindRoomsAct);
    if (Map.selections.size() == 2)
        actionManager->bindRoomsAct->setEnabled(true);
    else
        actionManager->bindRoomsAct->setEnabled(false);

    menu.exec(e->globalPos());
}

QPoint CMainWindow::mousePosInRenderer( QPoint pos ) {

    QPoint inFramePos = pos;

    if (menuBar()->isHidden() == false) {
        int height = menuBar()->height();
        inFramePos.setY( inFramePos.y() -  height);
    }

    return inFramePos;
}

bool CMainWindow::checkMouseSelection( QMouseEvent *e )
{
    unsigned int id;

    if (mouseState.delta( e->pos() ) <= 100) {
        if (renderer->doSelect( mousePosInRenderer( e->pos() ), id ) == true) {
            if (e->modifiers() & Qt::ControlModifier) {
                if (Map.selections.isSelected( id) == true)
                    Map.selections.unselect( id );
                else
                    Map.selections.select( id );
            } else
                Map.selections.exclusiveSelection( id );

            return true;
        }
    }

    return false;
}

void CMainWindow::mouseReleaseEvent( QMouseEvent *e )
{

    if (e->button() == Qt::LeftButton) {
        mouseState.LeftButtonPressed = false;
    } else {
        mouseState.RightButtonPressed = false;
    }

#if QT_VERSION >= 0x040200
    if (toolMode == MoveMode)
        renderer->setCursor(Qt::OpenHandCursor);
#endif
}

void CMainWindow::mouseMoveEvent( QMouseEvent *e)
{
  QPoint pos;
  int dist_x, dist_y;

  pos = e->pos();
  dist_x = pos.x() - mouseState.oldPos.x();
  dist_y = pos.y() - mouseState.oldPos.y();
/*
  print_debug(DEBUG_INTERFACE, "mouseEvent. LeftMouse %s, RightMouse %s. Dist_x %i, dist_y %i.",
      ON_OFF(LeftButtonPressed), ON_OFF(RightButtonPressed), dist_x, dist_y);
*/

    if (toolMode == MoveMode) {
        if (mouseState.LeftButtonPressed) {
        	renderer->setUserX( renderer->getUserX() + (float) dist_x / 10.0);
        	renderer->setUserY( renderer->getUserY() - (float) dist_y / 10.0);
            toggle_renderer_reaction();

            mouseState.oldPos = pos;
        } else if (mouseState.RightButtonPressed) {
        	renderer->setAngleX( renderer->getAngleX() + dist_y );
        	renderer->setAngleY( renderer->getAngleY() + dist_x );

            toggle_renderer_reaction();
            mouseState.oldPos = pos;
        }
    }
}

void CMainWindow::wheelEvent(QWheelEvent *e)
{
    int delta;

    delta = e->delta();

    renderer->setUserZ( renderer->getUserZ() + (delta / 120));

    toggle_renderer_reaction();
}

void CMainWindow::setMapMoveMode()
{
    setToolMode(MoveMode);
}

void CMainWindow::setDeleteMode()
{
    setToolMode(DeleteMode);
}

void CMainWindow::setSelectMode()
{
    setToolMode(SelectMode);
}

void CMainWindow::setToolMode(ToolMode mode)
{
    toolMode = mode;

    switch (mode) {
    case MoveMode:
        actionManager->mapMoveToolAct->setChecked(true);
#if QT_VERSION >= 0x040200
        renderer->setCursor(Qt::OpenHandCursor);
#endif
        // make sure it's off
        mouseState.RightButtonPressed = false;
        break;
    case DeleteMode:
        actionManager->deleteToolAct->setChecked(true);
        renderer->setCursor(Qt::CrossCursor);
        break;
    case SelectMode:
        renderer->setCursor(Qt::ArrowCursor);
        actionManager->selectToolAct->setChecked(true);
        break;
    }
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
        if (conf->isDatabaseModified()) {
                switch(QMessageBox::information(this, "Pandora",
                                        "The map contains unsaved changes\n"
                                        "Do you want to save the changes before exiting?",
                                        "&Save", "&Discard", "Cancel",
                                        0,      // Enter == button 0
                                        2)) { // Escape == button 2
        case 0: // Save clicked or Alt+S pressed or Enter pressed.
            actionManager->save();
            break;
        case 1: // Discard clicked or Alt+D pressed
            // don't save but exit
            break;
        case 2: // Cancel clicked or Escape pressed
            event->ignore();
            return;// don't exit
            break;
        }

    }

    if (conf->isConfigModified()) {
        switch(QMessageBox::information(this, "Pandora",
                                        "The configuration was changed\n"
                                        "Do you want to write it down on disc before exiting?",
                                        "&Save", "&Discard", "Cancel",
                                        0,      // Enter == button 0
                                        2)) { // Escape == button 2
        case 0: // Save clicked or Alt+S pressed or Enter pressed.
            conf->saveConfig();
            break;
        case 1: // Discard clicked or Alt+D pressed
            // don't save but exit
            break;
        case 2: // Cancel clicked or Escape pressed
            event->ignore();
            return;// don't exit
            break;
        }
    }

    event->accept();
    QApplication::quit();
}

