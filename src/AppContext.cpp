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

#include "AppContext.h"

#include "Engine/CEngine.h"
#include "Proxy/proxy.h"
#include "Map/CRoomManager.h"
#include "Engine/CStacksManager.h"
#include "Map/CTree.h"
#include "Utils/CConfigurator.h"
#include "Gui/mainwindow.h"
#include "Proxy/userfunc.h"

AppContext &AppContext::instance()
{
    static AppContext context;
    return context;
}

AppContext::AppContext()
{
    // Components are initialized later via setters or init methods
}

AppContext::~AppContext()
{
    // Note: Components are currently managed externally
    // In a future refactoring, this class could take ownership
    // and delete components in proper order
}

void AppContext::initRoomManager()
{
    if (!m_roomManager) {
        m_roomManager = new CRoomManager();
    }
}

void AppContext::initStacker()
{
    if (!m_stacker) {
        m_stacker = new CStacksManager();
    }
}

void AppContext::initNameMap()
{
    if (!m_nameMap) {
        m_nameMap = new CTree();
    }
}
