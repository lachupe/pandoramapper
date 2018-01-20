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

#include <QSet>


#include "CSelectionManager.h"
#include "utils.h"

#include "Map/CRoomManager.h"

#include "Gui/mainwindow.h"

#include "Proxy/proxy.h"

void CSelectionManager::select(unsigned int id)
{
    if (isSelected(id) == false) {
        selection.insert(id);
        print_debug(DEBUG_INTERFACE, "selecting %i", id);
        emit roomSelected( id );
    }

    toggle_renderer_reaction();
    renderer_window->update_status_bar();
}

void CSelectionManager::unselect(unsigned int id)
{
    selection.remove(id);
    toggle_renderer_reaction();
    renderer_window->update_status_bar();
}


void CSelectionManager::resetSelection()
{
    selection.clear();
    toggle_renderer_reaction();
    renderer_window->update_status_bar();
}

CSelectionManager::CSelectionManager() 
{
    selection.clear();
}

CSelectionManager::~CSelectionManager()
{
    selection.clear();
}


void CSelectionManager::exclusiveSelection(unsigned int id) 
{
    if (selection.contains(id)) 
        return;
    else {
        resetSelection();
        select(id);
    }   
}


//########################### CMouseState ########################

CMouseState::CMouseState() 
{
    LeftButtonPressed = false;
    RightButtonPressed = false;

    oldPos.setX(0);
    oldPos.setY(0);

    origPos.setX(0);
    origPos.setY(0);
}


int CMouseState::delta(QPoint pos)
{
    int x = origPos.x()-pos.x();
    int y = origPos.y()-pos.y();

    return x*x + y*y;
}
