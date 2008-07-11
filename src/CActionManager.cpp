
#include <QFileDialog>
#include <QMessageBox>


#include "defines.h"

#include "mainwindow.h"
#include "utils.h"
#include "CStacksManager.h"
#include "CEngine.h"
#include "userfunc.h"
#include "proxy.h"
#include "CActionManager.h"

#include "RoomEditDialog.h"
#include "ConfigWidget.h"
#include "SpellsDialog.h"
#include "CMovementDialog.h"
#include "CLogDialog.h"
#include "finddialog.h"
#include "CGroupCommunicator.h"

CActionManager::CActionManager(CMainWindow *parentWindow) 
{
    parent = parentWindow;

    /* creating actions and connecting them here */
    newAct = new QAction(tr("&Close"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Close current map"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));    
    
    openAct = new QAction(tr("&Load..."), this);
    openAct->setShortcut(tr("Ctrl+L"));
    openAct->setStatusTip(tr("Open an existing map"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    
    reloadAct = new QAction(tr("&Reload..."), this);
    reloadAct->setShortcut(tr("Ctrl+R"));
    reloadAct->setStatusTip(tr("Reload current map"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(reload()));
    
    saveAct = new QAction(tr("&Save..."), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save currently opened map"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    
    saveAsAct = new QAction(tr("Save As..."), this);
    saveAsAct->setStatusTip(tr("Save the map As"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    
    publishAct= new QAction(tr("Remove Secrets"), this);
    publishAct->setStatusTip(tr("Removes all secret exits and rooms behind them from the map"));
    connect(publishAct, SIGNAL(triggered()), this, SLOT(publish_map()));    
    
        
    quitAct =  new QAction(tr("&Exit..."), this);
    quitAct->setShortcut(tr("Ctrl+Q"));
    quitAct->setStatusTip(tr("Quit Pandora"));
    connect(quitAct, SIGNAL(triggered()), parent, SLOT(close()));
        
    /* Room menu */
    findAct = new QAction(tr("Find..."), this);
    findAct->setStatusTip(tr("Find matching rooms"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(find()));
    
    roomeditAct= new QAction(tr("Edit"), this);
    roomeditAct->setStatusTip(tr("View/Edit current Room's info"));
    connect(roomeditAct, SIGNAL(triggered()), this, SLOT(edit_current_room()));    
    
    deleteAct= new QAction(tr("Delete"), this);
    deleteAct->setStatusTip(tr("Deletes this room"));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(delete_room()));    

    deleteFullyAct= new QAction(tr("Delete Fully"), this);
    deleteFullyAct->setStatusTip(tr("Deletes this room and everything that leads to it, including doors in other rooms"));
    connect(deleteFullyAct, SIGNAL(triggered()), this, SLOT(deleteFully()));    

    selectionTypeAct= new QAction(tr("Select On Any Layer"), this);
    selectionTypeAct->setStatusTip(tr("Selection can either pick the rooms on your current layer or on any lLayer."));
    selectionTypeAct->setCheckable(true);
    selectionTypeAct->setChecked( conf->getSelectOAnyLayer() );
    connect(selectionTypeAct, SIGNAL(triggered()), this, SLOT( selectionType() ));    


    
    mergeAct= new QAction(tr("Merge"), this);
    mergeAct->setStatusTip(tr("Tries to merge two twin rooms"));
    connect(mergeAct, SIGNAL(triggered()), this, SLOT(merge_room()));    

    bindRoomsAct= new QAction(tr("Bind Rooms"), this);
    bindRoomsAct->setStatusTip(tr("Tries to connect two rooms"));
    connect(bindRoomsAct, SIGNAL(triggered()), this, SLOT(bindRooms()));    


    refreshAct= new QAction(tr("Refresh"), this);
    refreshAct->setStatusTip(tr("Switches to selected room and refreshes it"));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshRoom()));    

    
    moveRoomAct= new QAction(tr("Move"), this);
    moveRoomAct->setStatusTip(tr("Moves the room or rooms by given shift"));
    connect(moveRoomAct, SIGNAL(triggered()), parent, SLOT(moveRoomDialog()));    

    /* Tools menu */

    selectToolAct = new QAction(tr("Select tool"), this);
    selectToolAct->setStatusTip(tr("Select one or more rooms"));
    selectToolAct->setCheckable(true);
    connect(selectToolAct, SIGNAL(triggered()), parent, SLOT(setSelectMode()));

    mapMoveToolAct = new QAction(tr("Move map"), this);
    mapMoveToolAct->setStatusTip(tr("Move map around"));
    mapMoveToolAct->setCheckable(true);
    connect(mapMoveToolAct, SIGNAL(triggered()), parent, SLOT(setMapMoveMode()));
 
    deleteToolAct = new QAction(tr("Delete tool"), this);
    deleteToolAct->setStatusTip(tr("Delete tool"));
    deleteToolAct->setCheckable(true);
    connect(deleteToolAct, SIGNAL(triggered()), parent, SLOT(setDeleteMode()));

    toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(selectToolAct);
    toolsGroup->addAction(mapMoveToolAct);
    toolsGroup->addAction(deleteToolAct);
    selectToolAct->setChecked(true);


    /* Mapping menu */
    
    mappingAct= new QAction(tr("Mapping"), this);
    mappingAct->setCheckable(true);
    mappingAct->setChecked(false);
    mappingAct->setStatusTip(tr("Switch mapping mode on/off"));
    connect(mappingAct, SIGNAL(triggered()), this, SLOT(mapping_mode()));    
    
    automergeAct= new QAction(tr("AutoMerge"), this);
    automergeAct->setCheckable(true);
    automergeAct->setStatusTip(tr("Automatically merge twin (same name/desc) rooms"));
    connect(automergeAct, SIGNAL(triggered()), this, SLOT(automerge()));    
    
    angryLinkerAct= new QAction(tr("AngryLinker"), this);
    angryLinkerAct->setCheckable(true);
    angryLinkerAct->setStatusTip(tr("Auto-link neightbour rooms"));
    connect(angryLinkerAct, SIGNAL(triggered()), this, SLOT(angrylinker()));    


    duallinkerAct= new QAction(tr("DualLinker"), this);
    duallinkerAct->setCheckable(true);
    duallinkerAct->setStatusTip(tr("Binds the connection in the mapped room with the room you came from"));
    connect(duallinkerAct, SIGNAL(triggered()), this, SLOT(duallinker()));    
    
  
    
    /* Configuration menu bar */
    
    always_on_top_action= new QAction(tr("Always on Top"), this);
    always_on_top_action->setStatusTip(tr("Always on Top"));
    always_on_top_action->setCheckable(true);
    connect(always_on_top_action, SIGNAL(toggled(bool)), this, SLOT(alwaysOnTop(bool)), Qt::QueuedConnection);
    always_on_top_action->setChecked(conf->getAlwaysOnTop());
    
    
    
    emulationAct= new QAction(tr("Emulation Mode"), this);
    emulationAct->setStatusTip(tr("Offline MUME Emulation"));
    emulationAct->setCheckable(true);
    emulationAct->setChecked(false);
    connect(emulationAct, SIGNAL(triggered()), this, SLOT(emulation_mode()));    
    
    
    setupGeneralAct= new QAction(tr("General Settings ..."), this);
    setupGeneralAct->setStatusTip(tr("Edit general settings"));
    connect(setupGeneralAct, SIGNAL(triggered()), this, SLOT(generalSetting()) );    
    
    spellsAct= new QAction(tr("Spells Settings"), this);
    spellsAct->setStatusTip(tr("Spells Settings"));
    connect(spellsAct, SIGNAL(triggered()), this, SLOT(spellsSettings()) );    
    
    
    saveConfigAct= new QAction(tr("Save Configuration ..."), this);
    saveConfigAct->setStatusTip(tr("Save current configuration"));
    connect(saveConfigAct, SIGNAL(triggered()), this, SLOT(saveConfig()));    
    
    saveConfigAsAct= new QAction(tr("Save Configuration As ..."), this);
    saveConfigAsAct->setStatusTip(tr("Save current configuration to a different file"));
    connect(saveConfigAsAct, SIGNAL(triggered()), this, SLOT(saveAsConfig()));    
    
    loadConfigAct= new QAction(tr("Load Configuration"), this);
    loadConfigAct->setStatusTip(tr("Save current configuration to a different file"));
    connect(loadConfigAct, SIGNAL(triggered()), this, SLOT(loadConfig()));    
    
    showLogAct = new QAction(tr("&Show Log"), this);
    showLogAct->setStatusTip(tr("Show the application's Log file"));
    connect(showLogAct, SIGNAL(triggered()), this, SLOT(showLog() ));


    /* Help menu. */
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    
    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    // Additional Context menu actions
    gotoAct = new QAction(tr("Goto"), this);
    connect(gotoAct, SIGNAL(triggered()), this, SLOT(gotoAction()));
    
    
    // group Manager
    groupOffAct = new QAction(tr("Off"), this );
    groupOffAct->setCheckable(true);
    connect(groupOffAct, SIGNAL(toggled(bool)), this, SLOT(groupOff(bool)), Qt::QueuedConnection);

    groupClientAct = new QAction(tr("Client"), this );
    groupClientAct->setCheckable(true);
    connect(groupClientAct, SIGNAL(toggled(bool)), this, SLOT(groupClient(bool)), Qt::QueuedConnection);

    groupServerAct = new QAction(tr("Server"), this );
    groupServerAct->setCheckable(true);
    connect(groupServerAct, SIGNAL(toggled(bool)), this, SLOT(groupServer(bool)), Qt::QueuedConnection);

    groupManagerGroup = new QActionGroup(this);
    groupManagerGroup->addAction(groupOffAct);
    groupManagerGroup->addAction(groupClientAct);
    groupManagerGroup->addAction(groupServerAct);
    groupManagerTypeChanged( conf->getGroupManagerState() );
    
    groupShowHideAct = new QAction(tr("Show/Hide Manager"), this );
    connect(groupShowHideAct, SIGNAL(triggered()), this, SLOT(groupHide()), Qt::QueuedConnection);
    conf->setShowGroupManager( !conf->getShowGroupManager() );
    if (conf->getShowGroupManager() ) {
    	groupShowHideAct->setText("Hide");
    } else {
    	groupShowHideAct->setText("Show");
    }
    groupShowHideAct->trigger();
    
    groupSettingsAct = new QAction(tr("Settings"), this);
    connect(groupSettingsAct, SIGNAL(triggered()), this, SLOT(groupSettings()));
}

void CActionManager::groupManagerTypeChanged(int type)
{
	print_debug(DEBUG_GROUP, "Action: signal received groupManager type changed");
	
    if (type == CGroupCommunicator::Server)
    	groupServerAct->setChecked(true);
    if (type == CGroupCommunicator::Client)
    	groupClientAct->setChecked(true);
    if (type == CGroupCommunicator::Off)
    	groupOffAct->setChecked(true);
}


void CActionManager::groupOff(bool b)
{
    print_debug(DEBUG_INTERFACE, "Changing groupManager type to client");
	if (b)
		parent->getGroupManager()->setType(CGroupCommunicator::Off);
    print_debug(DEBUG_INTERFACE, "Done.");
}


void CActionManager::groupClient(bool b)
{
    print_debug(DEBUG_INTERFACE, "Changing groupManager type to client");
	if (b)
		parent->getGroupManager()->setType(CGroupCommunicator::Client);
    print_debug(DEBUG_INTERFACE, "Done.");
}


void CActionManager::groupServer(bool b)
{
    print_debug(DEBUG_INTERFACE, "Changing groupManager type to server");
	if (b)
		parent->getGroupManager()->setType(CGroupCommunicator::Server);
    print_debug(DEBUG_INTERFACE, "Done.");
}

void CActionManager::setShowGroupManager(bool b)
{
    print_debug(DEBUG_INTERFACE, "Changing the groupManager visibility");
	conf->setShowGroupManager(b);
	if (b) {
		printf("Showing\r\n");
    	groupShowHideAct->setText("Hide Manager");
        parent->setShowGroupManager(b);
	} else {
		printf("Hiding\r\n");
    	groupShowHideAct->setText("Show Manager");
        parent->setShowGroupManager(b);
	}
    print_debug(DEBUG_INTERFACE, "Done.");
}

void CActionManager::groupHide()
{
	setShowGroupManager( !conf->getShowGroupManager() );
}

void CActionManager::groupSettings()
{
	print_debug(DEBUG_GROUP, "Starting the dialog ...");
	
    if (!parent->groupDialog) {
        parent->groupDialog = new CGroupSettingsDialog(parent);
    }

    parent->groupDialog->run();
    parent->groupDialog->show();
    parent->groupDialog->raise();
    parent->groupDialog->activateWindow();

}


void CActionManager::bindRooms()
{
    CRoom *one, *two;
    int dir;

    if (Map.selections.size() != 2) {
        QMessageBox::critical(parent, "Failure", QString("You have to select two rooms to bind them."));
        return;
    }


    one = Map.getRoom( Map.selections.get(0) );
    two = Map.getRoom( Map.selections.get(1) );

    // roll over all dirs
    for (dir = 0; dir <= 5; dir++) 
        // and check if there are connections like undefined exits north-south etc
        if (one->isExitUndefined( dir) == true && two->isExitUndefined( reversenum( dir ) ) ) {
            // now test is the connection is geometrically right
            bool fits = false;

            if ( (dir == NORTH && one->getY() < two->getY()) ||
                 (dir == EAST && one->getX() < two->getX())  ||
                 (dir == WEST && one->getX() > two->getX())  ||
                 (dir == SOUTH && one->getY() > two->getY()) ||
                 (dir == UP && one->getZ() < two->getZ())    ||
                 (dir == DOWN && one->getZ() > two->getZ()) ) 
            {
                fits = true;
            }

            if (fits == true) {
                one->setExit(dir, two);
                if (conf->getDuallinker() == true)
                    two->setExit( reversenum( dir ), one);
                return;
            }
                
        }

    
    QMessageBox::critical(parent, "Failure", QString("No fitting exits found. Rooms are badly positioned or exits are not marked as Undefined."));
        

}

void CActionManager::edit_current_room()
{
    unsigned int id;

    if (Map.selections.isEmpty() == false) {
        id = Map.selections.getFirst();
    } else {
        if (stacker.amount() != 1) {
            QMessageBox::critical(parent, "Room Info Edit", QString("You are not in sync!"));
            return;
        } 
        id = stacker.first()->id;
    }
    
    parent->editRoomDialog( id );
}


void CActionManager::updateActionsSettings()
{
    conf->getDuallinker() ? duallinkerAct->setChecked(true) : duallinkerAct->setChecked(false);
    conf->getAngrylinker() ? angryLinkerAct->setChecked(true) : angryLinkerAct->setChecked(false);
    proxy->isMudEmulation() ? emulationAct->setChecked(true) : emulationAct->setChecked(false);
    engine->isMapping() ?  mappingAct->setChecked(true) : mappingAct->setChecked(false);
    conf->getAutomerge() ? automergeAct->setChecked(true) : automergeAct->setChecked(false);
}


void CActionManager::showLog()
{
    if (!parent->logdialog) {
        parent->logdialog = new CLogDialog(parent);
    }

    parent->logdialog->run();
    parent->logdialog->show();
    parent->logdialog->raise();
    parent->logdialog->activateWindow();
}


void CActionManager::spellsSettings()
{
    if (!parent->spells_dialog) {
        parent->spells_dialog = new SpellsDialog(parent);
    }

    parent->spells_dialog->run();
    parent->spells_dialog->show();
    parent->spells_dialog->raise();
    parent->spells_dialog->activateWindow();
}

void CActionManager::generalSetting()
{
    if (!parent->generalSettingsDialog) {
        parent->generalSettingsDialog = new ConfigWidget(parent);
    }

    parent->generalSettingsDialog->run();
    parent->generalSettingsDialog->show();
    parent->generalSettingsDialog->raise();
    parent->generalSettingsDialog->activateWindow();
}


void CActionManager::about()
{
    QMessageBox::about(parent, tr("About Pandora"),
            tr("<h2>Pandora MUME Mapper</h2>"
#ifdef SVN_REVISION
               "<p>SVN revision ") + QString::number(SVN_REVISION) + tr(
#endif
               "<p>Copyright &copy; 2003-2007 Azazello and contributors."
               "<p>This is an automatic mapper and database software for "
               "a game named MUME. It uses MUME's XML for parsing the "
               "output of the game to get user's position on the map and "
               "represents it in 3d view using OpenGL."
               "<p>Visit <a href=\"http://code.google.com/p/pandoramapper/\">code.google.com/p/pandoramapper/</a> for more information."));
}




void CActionManager::alwaysOnTop(bool set_on_top)
{
  print_debug(DEBUG_INTERFACE, "always_on_top called");

  Qt::WindowFlags flags = parent->windowFlags();
  if( set_on_top )
  {
    flags |= Qt::WindowStaysOnTopHint;
    print_debug(DEBUG_INTERFACE, "setting WindowStaysOnTopHint");
  } else {
    flags &= ~Qt::WindowStaysOnTopHint;
    print_debug(DEBUG_INTERFACE, "clearing WindowStaysOnTopHint");
  }
  parent->setWindowFlags(flags);
  parent->show();
  conf->setAlwaysOnTop(set_on_top);
  print_debug(DEBUG_INTERFACE, "Done.");
}



void CActionManager::newFile()
{
	// creates a new map. 
	// by now - just clears the existing one.

	if (conf->isDatabaseModified()) {
	        switch(QMessageBox::information(parent, "Pandora",
                    "The map contains unsaved changes\n"
                    "Do you want to save the changes before exiting?",
                    "&Save", "&Discard", "Cancel",
                    0,      // Enter == button 0
                    2)) { // Escape == button 2
        case 0: // Save clicked or Alt+S pressed or Enter pressed.
            save();
            break;
        case 1: // Discard clicked or Alt+D pressed
            // don't save but exit
            break;
        case 2: // Cancel clicked or Escape pressed
            return;// don't exit
            break;
        }    
    
    } 

	Map.reinit();  /* this one reinits Ctree structure also */
	stacker.reset();  /* resetting stacks */
	engine->clear();
	engine->setMapping(false);
	toggle_renderer_reaction();
}


void CActionManager::disable_online_actions()
{
    mappingAct->setEnabled(false);
    automergeAct->setEnabled(false);
    angryLinkerAct->setEnabled(false);
    duallinkerAct->setEnabled(false);
}


void CActionManager::enable_online_actions()
{
    mappingAct->setEnabled(true);
    automergeAct->setEnabled(true);
    angryLinkerAct->setEnabled(true);
    duallinkerAct->setEnabled(true);
}


void CActionManager::delete_room()
{
    if (Map.selections.size() > 1) {
        int ret = QMessageBox::warning(parent, tr("Pandora: Room's Deletion"),
                   tr("Do you really want to delete this room/rooms?"),
                   QMessageBox::Ok |  QMessageBox::Cancel,
                   QMessageBox::Ok);


        if (ret == QMessageBox::Cancel) 
            return;
    }
//    while (Map.selections.isEmpty() != true)
        userland_parser->parse_user_input_line("mdelete");
}

void CActionManager::deleteFully()
{
    int ret = QMessageBox::warning(parent, tr("Pandora: Room's Deletion"),
                   tr("Do you really want to delete this room/rooms?\n"
                       "Note that this deletion will also remove all\n"
                        "exits and doors leading to this room from all\n"
                        "other rooms"),
                   QMessageBox::Ok |  QMessageBox::Cancel,
                   QMessageBox::Ok);

    if (ret == QMessageBox::Cancel) 
        return;
//    while (Map.selections.isEmpty() == false)
        userland_parser->parse_user_input_line("mdelete remove");
}


void CActionManager::merge_room()
{
    userland_parser->parse_user_input_line("mmerge");
}

void CActionManager::open()
{
  QString s = QFileDialog::getOpenFileName(
                    parent,
                    "Choose a database",
                    "database/",
                    "XML files (*.xml)");    
  char data[MAX_STR_LEN];
    
  print_debug(DEBUG_XML, "User wants to load the database from the file: %s", qPrintable(s));
  strcpy(data, qPrintable(s));
    
  if (!s.isEmpty()) { 
    usercmd_mload(0, 0,  data, data);  
  }  
}

void CActionManager::reload()
{
    userland_parser->parse_user_input_line("mload");
}

void CActionManager::save()
{
    userland_parser->parse_user_input_line("msave");
    QMessageBox::information(parent, "Saving...", "Saved!\n", QMessageBox::Ok);
}

void CActionManager::saveAs()
{
  char data[MAX_STR_LEN];
  
  QString s = QFileDialog::getSaveFileName(
                    parent,
                    "Choose a filename to save under",
                    "database/",
                    "XML database files (*.xml)");
                    
  strcpy(data, qPrintable(s));
    
  if (!s.isEmpty()) { 
    usercmd_msave(0, 0,  data, data);  
    QMessageBox::information(parent, "Saving...", "Saved!\n", QMessageBox::Ok);
  }  
}


void CActionManager::mapping_mode()
{
    if (mappingAct->isChecked()) 
    {
        userland_parser->parse_user_input_line("mmap on");
    } else {
        userland_parser->parse_user_input_line("mmap off");
    }
}

void CActionManager::refreshRoom()
{
    QString command;
    CRoom *r;

    if (Map.selections.isEmpty() == true) {
        if (stacker.amount() != 1) {
            QMessageBox::critical(parent, "Pandora",
                              QString("You have to be in sync or select just one room!"));
            return;
        }        
        r = stacker.first();
    } else {
        if (Map.selections.size() != 1) {
            QMessageBox::critical(parent, "Pandora",
                              QString("You have to select just one room!"));
            return;
        }

        r = Map.getRoom( Map.selections.getFirst() );
    }

    command = QString("mgoto %1").arg(r->id);
    userland_parser->parse_user_input_line( (const char *) command.toAscii() );
    userland_parser->parse_user_input_line("mrefresh");
}


void CActionManager::selectionType()
{
    // set_conf_mod() will take care of switching the state of the Action itself
    conf->setSelectOAnyLayer( !conf->getSelectOAnyLayer() );
}


void CActionManager::automerge()
{
    if (automergeAct->isChecked()) 
    {
        userland_parser->parse_user_input_line("mautomerge on");
    } else {
        userland_parser->parse_user_input_line("mautomerge off");
    }
}

void CActionManager::angrylinker()
{
    if (angryLinkerAct->isChecked()) 
    {
        userland_parser->parse_user_input_line("mangrylinker on");
    } else {
        userland_parser->parse_user_input_line("mangrylinker off");
    }
}


void CActionManager::duallinker()
{
    if (angryLinkerAct->isChecked()) 
    {
        userland_parser->parse_user_input_line("mduallinker on");
    } else {
        userland_parser->parse_user_input_line("mduallinker off");
    }
}

void CActionManager::saveConfig()
{
    conf->saveConfig();
    QMessageBox::information(parent, "Saving...", "Saved!\n", QMessageBox::Ok);
}

void CActionManager::saveAsConfig()
{
    QString s = QFileDialog::getSaveFileName(
                    parent,
                    "Choose a filename to save under",
                    "configs/",
                    "XML config files (*.xml)");
    if (s.isEmpty())
        return;
    conf->saveConfigAs("", s.toAscii());
    QMessageBox::information(parent, "Saving...", "Saved!\n", QMessageBox::Ok);
}

void CActionManager::loadConfig()
{
  QString s = QFileDialog::getOpenFileName(
                    parent,
                    "Choose another configuration file to load",
                    "configs/",
                    "XML config files (*.xml)");    
  if (s.isEmpty())
        return;

  conf->loadConfig("", s.toAscii());
  QMessageBox::information(parent, "Pandora", "Loaded!\n", QMessageBox::Ok);

}    

void CActionManager::emulation_mode()
{
    if (emulationAct->isChecked()) {
        /* cannot turn it on if we have anyone connected to us already */
        /* tricky check - if online_actions are on, then we have connections */
        if (mappingAct->isChecked()) {
            emulationAct->setChecked(false); 
            QMessageBox::critical(parent, "Pandora",
                              QString("You have to disconnect from the game first!"));
            return;        
        }
        proxy->setMudEmulation( true );
        engine->setPrompt("-->");
        stacker.put(1);
        stacker.swap();
    } else {
        proxy->setMudEmulation( false );
    
    }

}


void CActionManager::publish_map()
{

    if (mappingAct->isChecked()) {
        emulationAct->setChecked(false); 
        QMessageBox::critical(parent, "Pandora",
                          QString("You have to disconnect from the game first!"));
        return;        
    }

    Map.clearAllSecrets();

    print_debug(DEBUG_INTERFACE && DEBUG_ROOMS,"Finished removing secrets from the map!\r\n");
    //    QMessageBox::information(parent, "Removing secrets...", "Done!\n", QMessageBox::Ok);
}



void CActionManager::gotoAction() {
    userland_parser->parse_user_input_line("mgoto");
}

void CActionManager::find()
{
    if (!parent->findDialog) {
        parent->findDialog = new FindDialog(parent);
    }
    parent->findDialog->show();
    parent->findDialog->activateWindow();
}
