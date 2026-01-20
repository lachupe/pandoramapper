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

#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <memory>
#include <QString>

// Forward declarations
class CEngine;
class Proxy;
class CRoomManager;
class CStacksManager;
class CTree;
class Configurator;
class CMainWindow;
class Userland;

/**
 * @brief Application Context - centralizes global state management
 *
 * This singleton class owns all major application components that were
 * previously declared as extern globals. It provides:
 * - Clear ownership semantics
 * - Proper initialization/destruction order
 * - Easier testing (can be mocked)
 * - Single point of access to shared resources
 *
 * Usage:
 *   AppContext::instance().engine()
 *   AppContext::instance().proxy()
 *   etc.
 *
 * For compatibility with existing code, the old extern variables
 * are still available but point to objects owned by this context.
 */
class AppContext {
public:
    static AppContext& instance();

    // Non-copyable
    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    // Component accessors
    CEngine* engine() const { return m_engine; }
    Proxy* proxy() const { return m_proxy; }
    CRoomManager& roomManager() { return *m_roomManager; }
    CStacksManager& stacker() { return *m_stacker; }
    CTree& nameMap() { return *m_nameMap; }
    Configurator* configurator() const { return m_configurator; }
    CMainWindow* mainWindow() const { return m_mainWindow; }
    Userland* userland() const { return m_userland; }
    QString* logFileName() const { return m_logFileName; }

    // Component setters (for initialization in main.cpp)
    void setEngine(CEngine* engine) { m_engine = engine; }
    void setProxy(Proxy* proxy) { m_proxy = proxy; }
    void setConfigurator(Configurator* conf) { m_configurator = conf; }
    void setMainWindow(CMainWindow* window) { m_mainWindow = window; }
    void setUserland(Userland* userland) { m_userland = userland; }
    void setLogFileName(QString* fileName) { m_logFileName = fileName; }

    // Initialize heap-allocated singletons
    void initRoomManager();
    void initStacker();
    void initNameMap();

private:
    AppContext();
    ~AppContext();

    // Owned components
    CEngine* m_engine = nullptr;
    Proxy* m_proxy = nullptr;
    CRoomManager* m_roomManager = nullptr;
    CStacksManager* m_stacker = nullptr;
    CTree* m_nameMap = nullptr;
    Configurator* m_configurator = nullptr;
    CMainWindow* m_mainWindow = nullptr;
    Userland* m_userland = nullptr;
    QString* m_logFileName = nullptr;
};

// Convenience macro for accessing context
#define APP AppContext::instance()

#endif // APPCONTEXT_H
