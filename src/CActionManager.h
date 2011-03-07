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

// ActionManager
#ifndef CACTIONMANAGER_H
#define CACTIONMANAGER_H


#include <QMainWindow>

class CMainWindow;
class QActionGroup;

class CActionManager : public QObject {
    Q_OBJECT

    CMainWindow *parent;
public:
    CActionManager(CMainWindow *parent);

    QAction       *newAct;
    QAction       *openAct;
    QAction       *reloadAct;
    QAction       *saveAct;
    QAction       *saveAsAct;
    QAction       *quitAct;
    QAction       *publishAct;

    QAction       *mappingAct;
    QAction       *automergeAct;
    QAction       *angryLinkerAct;
    QAction       *duallinkerAct;

    QActionGroup  *toolsGroup;
    QAction       *selectToolAct;
    QAction       *mapMoveToolAct;
    QAction       *deleteToolAct;

    QAction       *findAct;
    QAction       *roomeditAct;
    QAction       *deleteAct;
    QAction       *mergeAct;
    QAction       *moveRoomAct;
    QAction       *selectionTypeAct;
    QAction       *refreshAct;
    QAction       *bindRoomsAct;

    QAction       *always_on_top_action;
    QAction       *saveConfigAct;
    QAction       *saveConfigAsAct;
    QAction       *loadConfigAct;
    QAction       *setupGeneralAct;
    QAction       *spellsAct;

    QAction       *showLogAct;

    QAction       *gotoAct;
    QAction       *deleteFullyAct;
    
    QAction       *aboutQtAct;
    QAction       *aboutAct;
    
    QAction       *emulationAct;
    
    QActionGroup  *groupManagerGroup;
    QAction		  *groupOffAct;
    QAction		  *groupClientAct;
    QAction		  *groupServerAct;
    QAction		  *groupShowHideAct;
    QAction		  *groupSettingsAct;
    QAction		  *groupClearSpellsAct;
    
    
    void setShowGroupManager(bool b);


public slots:
    void disable_online_actions();
    void enable_online_actions();
    void updateActionsSettings();
    
    
    void alwaysOnTop(bool);
    void newFile();
    void open();
    void reload();
    void save();
    void saveAs();

    void showLog();
    
    void mapping_mode();
    void automerge();
    void angrylinker();
    void duallinker();

    
    void delete_room(); 
    void merge_room(); 
    void selectionType();
    void refreshRoom();
    void bindRooms();
    
    void saveConfig();
    void saveAsConfig();
    void loadConfig();
    
    void generalSetting();
    void spellsSettings();
    void edit_current_room();
    
    void emulation_mode();  
    void publish_map();
    void about();
    void gotoAction();
    void deleteFully(); // delete with remove flag
    
    void groupOff(bool);
    void groupClient(bool);
    void groupServer(bool);
    void groupHide();
    void groupSettings();
    void groupManagerHides() { setShowGroupManager(false); }
    void groupManagerTypeChanged(int);
    void groupClearSpells();


private slots:
    void find();
};


#endif
