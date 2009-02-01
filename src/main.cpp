/* automapper/database module for powwow. Part of the Pandora Project (c) 2003 */

#include <QApplication>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QThread>
#include <QMutex>
#include <QObject>
#include <QSplashScreen>
#include <QRect>
#include <QDesktopWidget>

#include "defines.h"
#include "CRoom.h"
#include "CConfigurator.h"


#include "CRoomManager.h"
#include "xml2.h"

#include "CDispatcher.h"
#include "mainwindow.h"
#include "CStacksManager.h"
#include "proxy.h"
#include "utils.h"
#include "CEngine.h"
#include "userfunc.h"


#ifdef Q_OS_MACX
#include <CoreFoundation/CoreFoundation.h>
#endif

QString *logFileName;

void print_usage()
{
  printf("Usage: pandora <options>\r\n");
  printf("Options:\r\n\r\n");
  printf("  --help / -h                   - this helpfile.\r\n");
  printf("  --base / -b  <filename>       - override the database file.\r\n");
  printf("  --localport / -lp  <port>     - override the local port number.\r\n");
  printf("  --hostname / -hn  <host>      - override the remote (game) host name.\r\n");
  printf("  --remoteport / -rp  <port>    - override the remote (game) port number.\r\n");
  printf("  --emulate / -e  <port>        - emulate mud environment (disabled).\r\n");
  printf("  --config / -c  <configfile>   - load config file.\r\n");

  printf("\r\n");
}

int main(int argc, char *argv[])
{
    int i;
    const char *resPath = 0;
    char    override_base_file[MAX_STR_LEN] = "";
    int     override_local_port = 0;
    char    override_remote_host[MAX_STR_LEN] = "";
    int     override_remote_port = 0;
    char    configfile[MAX_STR_LEN] = "configs/mume.xml";
    int     default_local_port = 3000;
    int     default_remote_port = 4242;
    int     mud_emulation = 0;

#ifdef Q_OS_MACX
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef,
						  kCFURLPOSIXPathStyle);
    const char *appPath = CFStringGetCStringPtr(macPath,
						CFStringGetSystemEncoding());
    resPath = (char *)malloc(strlen(appPath)+25);
    strcpy(resPath, appPath);
    strcat(resPath, "/Contents/Resources/");

    char    default_base_file[MAX_STR_LEN] = "databases/base.xml";
    char    default_remote_host[MAX_STR_LEN] = "";
    strcpy(configfile, "configs/default.conf");

    CFRelease(pluginRef);
    CFRelease(macPath);

#else
    resPath = "";
    char    default_base_file[MAX_STR_LEN] = "database/mume.xml";
    char    default_remote_host[MAX_STR_LEN] = "129.241.210.221";
#endif
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication app( argc, argv );

    QPixmap pixmap("images/logo.png");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();

    splash->showMessage("Loading configuration and database...");


    for (i=1; i < argc; i++) {

      if ((strcmp(argv[i], "--config") == 0) || ( strcmp(argv[i], "-c") == 0))
      {
        if (i == argc) {
          printf("Too few arguments. Missing config file name.\r\n");
          print_usage();
          exit(1);
        }
        i++;

        strcpy(configfile, argv[i]);
	resPath = ""; // obviously the user has an own config file - including the path
      }

      if ((strcmp(argv[i], "--emulate") == 0) || ( strcmp(argv[i], "-e") == 0))
      {
        printf("Pandora: Starting in MUD emulation mode.\r\n");
        mud_emulation = 1;
      }

      if ((strcmp(argv[i], "--base") == 0) || ( strcmp(argv[i], "-b") == 0))
      {
        if (i == argc) {
          printf("Too few arguments. Missing database.\r\n");
          print_usage();
          exit(1);
        }
        i++;
        strcpy(override_base_file, argv[i]); // overriding the database file is possible even with default config file
      }

      if ((strcmp(argv[i], "--hostname") == 0) || ( strcmp(argv[i], "-hn") == 0))
      {
        if (i == argc) {
          printf("Too few arguments. Wrong hostname given.\r\n");
          print_usage();
          exit(1);
        }
        i++;
        strcpy(override_remote_host, argv[i]);
      }

      if ((strcmp(argv[i], "--localport") == 0) || ( strcmp(argv[i], "-lp") == 0))
      {
        if (i == argc) {
          printf("Too few arguments. Missing localport.\r\n");
          print_usage();
          exit(1);
        }
        i++;
        override_local_port = atoi(argv[i]);
      }

      if ((strcmp(argv[i], "--remoteport") == 0) || ( strcmp(argv[i], "-rp") == 0))
      {
        if (i == argc) {
          printf("Too few arguments. Missing targetport.\r\n");
          print_usage();
          exit(1);
        }
        i++;
        override_remote_port = atoi(argv[i]);
      }


      if ((strcmp(argv[i], "--help") == 0) || ( strcmp(argv[i], "-h") == 0))
      {
        print_usage();
        exit(1);
      }

    }



    /* set analyzer engine defaults */
    //engine_init();
    splash->showMessage(QString("Loading the configuration ") + configfile);
    print_debug(DEBUG_SYSTEM, "Using config file : %s.", configfile);
    conf = new Cconfigurator();
    conf->loadConfig(resPath, configfile);
    print_debug(DEBUG_SYSTEM, "starting up...");


    if (override_base_file[0] != 0) {
      conf->setBaseFile(override_base_file);
    } else if ( conf->getBaseFile() == "") {
      conf->setBaseFile(default_base_file);
    }
    print_debug(DEBUG_SYSTEM, "Using database file : %s.", (const char*) conf->getBaseFile() );

    if (override_remote_host[0] != 0) {
      conf->setRemoteHost(override_remote_host);
    } else if ( conf->getRemoteHost().isEmpty() ) {
      conf->setRemoteHost(default_remote_host);
    }
    print_debug(DEBUG_SYSTEM, "Using target hostname : %s.", (const char*) conf->getRemoteHost() );

    if (override_local_port != 0) {
      conf->setLocalPort(override_local_port);
    } else if ( conf->getLocalPort() == 0) {
      conf->setLocalPort(default_local_port);
    }
    print_debug(DEBUG_SYSTEM, "Using local port : %i.", conf->getLocalPort());

    if (override_remote_port != 0) {
      conf->setRemotePort(override_remote_port);
    } else if (conf->getRemotePort() == 0) {
      conf->setRemotePort(default_remote_port);
    }
    print_debug(DEBUG_SYSTEM, "Using target port : %i.", conf->getRemotePort());

    conf->setConfigModified( false );

    splash->showMessage("Starting Analyzer and Proxy...");
    engine = new CEngine();
    proxy = new Proxy();

    //splash->showMessage("Loading the database, please wait...");
    //print_debug(DEBUG_SYSTEM, "Loading the database ... ");
    //xml_readbase( conf->get_base_file() );
    //print_debug(DEBUG_SYSTEM, "Successfuly loaded %i rooms!", Map.size());

    /* special init for the mud emulation */
    if (mud_emulation) {
      print_debug(DEBUG_SYSTEM, "Starting in MUD emulation mode...");

      engine->setPrompt("-->");
      stacker.put(1);
      stacker.swap();
    }

    proxy->setMudEmulation( mud_emulation );


    print_debug(DEBUG_SYSTEM, "Starting renderer ...\n");

    if ( !QGLFormat::hasOpenGL() ) {
        qWarning( "This system has no OpenGL support. Quiting." );
        return -1;
    }

    QRect rect = app.desktop()->availableGeometry(-1);
    if (conf->getWindowRect().x() == 0 || conf->getWindowRect().x() >= rect.width() ||
        conf->getWindowRect().y() >= rect.height() ) {
        print_debug(DEBUG_SYSTEM && DEBUG_INTERFACE, "Autosettings for window size and position");
        int x, y, height, width;

        x = rect.width() / 3 * 2;
        y = 0;
        height = rect.height() / 3;
        width = rect.width() - x;

        conf->setWindowRect( x, y, width, height);
    }

    renderer_window = new CMainWindow( 0 );

    QGLFormat f;
    f.setDoubleBuffer( true );
    f.setDirectRendering( true );
    f.setRgba( true );
    f.setDepth( true );
    f.setSampleBuffers( true );

    QGLFormat::setDefaultFormat( f );


    renderer_window->show();

    splash->finish(renderer_window);
    delete splash;

    printf("Main Thread ID: %i\r\n", (int) QThread::currentThreadId ());

    proxy->init();
    proxy->start();
    QObject::connect(proxy, SIGNAL(startEngine()), engine, SLOT(slotRunEngine()), Qt::QueuedConnection );
    QObject::connect(proxy, SIGNAL(startRenderer()), renderer_window->renderer, SLOT(display()), Qt::QueuedConnection);

    userland_parser->parse_user_input_line("mload");

    return app.exec();
}
