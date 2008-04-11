TEMPLATE = app
OBJECTS_DIR = obj
MOC_DIR = moc
CONFIG += qt \
    opengl \
    warn_on \
    thread

# CONFIG += debug
CONFIG += release
QT += xml \
    opengl \
    gui \
    network
FORMS += src/groupmanagersettings.ui \
    src/configedit.ui \
    src/finddialog.ui \
    src/logdialog.ui \
    src/movementdialog.ui \
    src/roomedit.ui \
    src/spellsdialog.ui
HEADERS += src/CGroupSettingsDialog.h \
    src/CGroupChar.h \
    src/CGroupCommunicator.h \
    src/CGroupClient.h \
    src/CGroupServer.h \
    src/CGroup.h \
    src/CActionManager.h \
    src/CConfigurator.h \
    src/CDispatcher.h \
    src/CEngine.h \
    src/CEvent.h \
    src/CFrustum.h \
    src/CLogDialog.h \
    src/CMovementDialog.h \
    src/CRegion.h \
    src/CRoom.h \
    src/CRoomManager.h \
    src/CSelectionManager.h \
    src/CSquare.h \
    src/CStacksManager.h \
    src/CTree.h \
    src/ConfigWidget.h \
    src/RoomEditDialog.h \
    src/SpellsDialog.h \
    src/defines.h \
    src/exits.h \
    src/mainwindow.h \
    src/proxy.h \
    src/renderer.h \
    src/userfunc.h \
    src/utils.h \
    src/xml2.h \
    src/finddialog.h
SOURCES += src/CGroupSettingsDialog.cpp \
    src/CGroupChar.cpp \
    src/CGroupCommunicator.cpp \
    src/CGroupClient.cpp \
    src/CGroupServer.cpp \
    src/CGroup.cpp \
    src/CActionManager.cpp \
    src/CDispatcher.cpp \
    src/CEngine.cpp \
    src/CFrustum.cpp \
    src/CLogDialog.cpp \
    src/CMovementDialog.cpp \
    src/CRoom.cpp \
    src/CRoomManager.cpp \
    src/CSelectionManager.cpp \
    src/CSquare.cpp \
    src/CStacksManager.cpp \
    src/CTree.cpp \
    src/ConfigWidget.cpp \
    src/Regions.cpp \
    src/RoomEditDialog.cpp \
    src/SpellsDialog.cpp \
    src/configurator.cpp \
    src/exits.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/proxy.cpp \
    src/renderer.cpp \
    src/userfunc.cpp \
    src/utils.cpp \
    src/finddialog.cpp \
    src/xml2.cpp
TARGET = release/pandora
macx:LIBS += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
win32:LIBS += -lwsock32
unix:LIBS += -lm

# !debug:unix:QMAKE_POST_LINK = strip \
# $(TARGET)
# SVN_REVISION = $$system("svn info | grep Revision | sed s/Revision:\ //")
# !isEmpty(SVN_REVISION):DEFINES += SVN_REVISION=108
# CFLAGS_VAR = $$system(pkg-config --cflags OGRE)
# CLIBS_VAR = $$system(pkg-config --libs OGRE)
QMAKE_CXXFLAGS_RELEASE += -O2 \
    -pipe \
    $$CFLAGS_VAR
QMAKE_CXXFLAGS_DEBUG += $$CFLAGS_VAR
