TEMPLATE = app
OBJECTS_DIR = obj
MOC_DIR = moc
UI_DIR = ui
CONFIG += qt \
    opengl \
    warn_on \
    thread

#CONFIG += qt_integration_plugin
CONFIG += debug_and_release
    
QT += xml \
    opengl \
    gui \
    network \
    core
FORMS += src/configedit.ui \
    src/finddialog.ui \
    src/groupmanagersettings.ui \
    src/logdialog.ui \
    src/movementdialog.ui \
    src/roomedit.ui \
    src/spellsdialog.ui \
    src/groupmanagersettings.ui \
    src/configedit.ui \
    src/finddialog.ui \
    src/logdialog.ui \
    src/movementdialog.ui \
    src/roomedit.ui \
    src/spellsdialog.ui
HEADERS += src/CTimers.h \
	src/patterns.h \
    src/CCommandQueue.h \
    src/CActionManager.h \
    src/CConfigurator.h \
    src/CDispatcher.h \
    src/CEngine.h \
    src/CEvent.h \
    src/CFrustum.h \
    src/CGroup.h \
    src/CGroupChar.h \
    src/CGroupClient.h \
    src/CGroupCommunicator.h \
    src/CGroupServer.h \
    src/CGroupSettingsDialog.h \
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
    src/finddialog.h \
    src/mainwindow.h \
    src/proxy.h \
    src/renderer.h \
    src/userfunc.h \
    src/utils.h \
    src/xml2.h
SOURCES += src/CTimers.cpp \
	src/patterns.cpp \
    src/CActionManager.cpp \
    src/CConfigurator.cpp \
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
    src/finddialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/proxy.cpp \
    src/renderer.cpp \
    src/userfunc.cpp \
    src/utils.cpp \
    src/xml2.cpp \
    src/CGroupSettingsDialog.cpp \
    src/CGroupChar.cpp \
    src/CGroupCommunicator.cpp \
    src/CGroupClient.cpp \
    src/CGroupServer.cpp \
    src/CGroup.cpp
TARGET = pandora

unix:!macx{
	DESTDIR = release/ 
}

macx:LIBS += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
win32:LIBS += -lwsock32
unix:LIBS += -lm

#QMAKE_CXXFLAGS_RELEASE += -O2 \
#    -pipe \
#    $$CFLAGS_VAR
#QMAKE_CXXFLAGS_DEBUG += $$CFLAGS_VAR
