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

/* automapper/database module for powwow. Part of the Pandora Project (c) 2003 */

#include <QApplication>

#include <QThread>
#include <QMutex>
#include <QObject>
#include <QSplashScreen>
#include <QRect>
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QString>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "defines.h"

#include "CConfigurator.h"
#include "xml2.h"
#include "utils.h"

#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

#include "Proxy/userfunc.h"
#include "Proxy/CDispatcher.h"
#include "Proxy/proxy.h"

#include "Gui/mainwindow.h"

#include "Engine/CStacksManager.h"
#include "Engine/CEngine.h"

#ifdef Q_OS_MACX
#include <CoreFoundation/CoreFoundation.h>
#endif

QString *logFileName;

int main(int argc, char *argv[])
{
    QString resPath;
    QString override_base_file;
    int override_local_port = 0;
    QString override_remote_host;
    int override_remote_port = 0;
    QString configfile = "mume.ini";
    const int default_local_port = 3000;
    const int default_remote_port = 4242;
    bool mud_emulation = false;

#ifdef Q_OS_MACX
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    const char *appPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    resPath = QString(appPath) + "/Contents/Resources/";

    QString default_base_file = "mume.xml";
    QString default_remote_host;
    configfile = "configs/default.conf";

    CFRelease(pluginRef);
    CFRelease(macPath);

#else
    resPath = "";
    QString default_base_file = "mume.xml";
    QString default_remote_host = "129.241.210.221";
#endif
    QApplication app(argc, argv);
    app.setApplicationName("PandoraMapper");
    app.setApplicationVersion(QString::number(SVN_REVISION));

    // Set up command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Pandora MUME mapper");
    parser.addHelpOption();

    QCommandLineOption configOption(QStringList() << "c" << "config", "Load config file.", "configfile");
    QCommandLineOption baseOption(QStringList() << "b" << "base", "Override the database file.", "filename");
    QCommandLineOption localPortOption(QStringList() << "lp" << "localport", "Override the local port number.", "port");
    QCommandLineOption hostOption(QStringList() << "hn" << "hostname", "Override the remote (game) host name.", "host");
    QCommandLineOption remotePortOption(QStringList() << "rp" << "remoteport",
                                        "Override the remote (game) port number.", "port");
    QCommandLineOption emulateOption(QStringList() << "e" << "emulate", "Emulate MUD environment.");

    parser.addOption(configOption);
    parser.addOption(baseOption);
    parser.addOption(localPortOption);
    parser.addOption(hostOption);
    parser.addOption(remotePortOption);
    parser.addOption(emulateOption);

    parser.process(app);

    if (parser.isSet(configOption)) {
        configfile = parser.value(configOption);
        resPath = "";  // user has own config file - including the path
    }
    if (parser.isSet(emulateOption)) {
        printf("Pandora: Starting in MUD emulation mode.\r\n");
        mud_emulation = true;
    }
    if (parser.isSet(baseOption)) {
        override_base_file = parser.value(baseOption);
    }
    if (parser.isSet(hostOption)) {
        override_remote_host = parser.value(hostOption);
    }
    if (parser.isSet(localPortOption)) {
        override_local_port = parser.value(localPortOption).toInt();
    }
    if (parser.isSet(remotePortOption)) {
        override_remote_port = parser.value(remotePortOption).toInt();
    }

    QPixmap pixmap("images/logo.png");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();

    splash->showMessage("Loading configuration and database...");

    /* set analyzer engine defaults */
    // engine_init();
    splash->showMessage(QString("Loading the configuration ") + configfile);
    conf = new Configurator();
    conf->loadConfig(resPath.toUtf8(), configfile.toUtf8());
    print_debug(DEBUG_SYSTEM, "starting up...");

    if (!override_base_file.isEmpty()) {
        conf->setBaseFile(override_base_file.toUtf8());
    } else if (conf->getBaseFile().isEmpty()) {
        conf->setBaseFile(default_base_file.toUtf8());
    }
    print_debug(DEBUG_SYSTEM, "Using database file : %s.", conf->getBaseFile().constData());

    if (!override_remote_host.isEmpty()) {
        conf->setRemoteHost(override_remote_host.toUtf8());
    } else if (conf->getRemoteHost().isEmpty()) {
        conf->setRemoteHost(default_remote_host.toUtf8());
    }
    print_debug(DEBUG_SYSTEM, "Using target hostname : %s.", conf->getRemoteHost().constData());

    if (override_local_port != 0) {
        conf->setLocalPort(override_local_port);
    } else if (conf->getLocalPort() == 0) {
        conf->setLocalPort(default_local_port);
    }
    print_debug(DEBUG_SYSTEM, "Using local port : %i.", conf->getLocalPort());

    if (override_remote_port != 0) {
        conf->setRemotePort(override_remote_port);
    } else if (conf->getRemotePort() == 0) {
        conf->setRemotePort(default_remote_port);
    }
    print_debug(DEBUG_SYSTEM, "Using target port : %i.", conf->getRemotePort());

    conf->setConfigModified(false);

    splash->showMessage("Starting Analyzer and Proxy...");
    engine = new CEngine();
    proxy = new Proxy();

    // splash->showMessage("Loading the database, please wait...");
    // print_debug(DEBUG_SYSTEM, "Loading the database ... ");
    // xml_readbase( conf->get_base_file() );
    // print_debug(DEBUG_SYSTEM, "Successfuly loaded %i rooms!", Map.size());

    /* special init for the mud emulation */
    if (mud_emulation) {
        print_debug(DEBUG_SYSTEM, "Starting in MUD emulation mode...");

        engine->setPrompt("-->");
        stacker.put(1);
        stacker.swap();
    }

    proxy->setMudEmulation(mud_emulation);

    print_debug(DEBUG_SYSTEM, "Starting renderer ...\n");

    // Set up OpenGL format using QSurfaceFormat (replaces deprecated QGLFormat)
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    // Keep the default framebuffer opaque; scene blending still works without alpha.
    format.setAlphaBufferSize(0);
    // Request compatibility profile for legacy OpenGL functions (GL_SELECT mode for picking)
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setVersion(2, 1);  // OpenGL 2.1 compatibility
    if (conf->getMultisampling()) {
        format.setSamples(4);
    }
    QSurfaceFormat::setDefaultFormat(format);

    QRect rect = QGuiApplication::primaryScreen()->availableGeometry();
    if (conf->getWindowRect().x() == 0 || conf->getWindowRect().x() >= rect.width() ||
        conf->getWindowRect().y() >= rect.height()) {
        print_debug(DEBUG_SYSTEM && DEBUG_INTERFACE, "Autosettings for window size and position");
        int x, y, height, width;

        x = rect.width() / 3 * 2;
        y = 0;
        height = rect.height() / 3;
        width = rect.width() - x;

        conf->setWindowRect(x, y, width, height);
    }

    renderer_window = new CMainWindow;

    renderer_window->show();

    splash->finish(renderer_window);
    delete splash;

    proxy->start();
    QObject::connect(proxy, SIGNAL(startEngine()), engine, SLOT(slotRunEngine()), Qt::QueuedConnection);
    QObject::connect(proxy, SIGNAL(startRenderer()), renderer_window->renderer, SLOT(display()), Qt::QueuedConnection);

    userland_parser->parse_user_input_line("mload");

    return app.exec();
}
