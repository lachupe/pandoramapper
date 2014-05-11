TEMPLATE = app

OBJECTS_DIR = obj

MOC_DIR = moc

UI_DIR = ui

RESOURCES = resources/pandora.qrc
RC_FILE = resources/pandora.rc

CONFIG += qt \
    opengl \
    warn_on \
    thread


CONFIG += debug_and_release
    
QT += xml \
    opengl \
    gui \
    network \
    core \
    widgets

DEFINES += NOMINMAX

INCLUDEPATH = src src/Utils protobuf/include
	
FORMS += src/Ui/configedit.ui \
    src/Ui/finddialog.ui \
    src/Ui/groupmanagersettings.ui \
    src/Ui/logdialog.ui \
    src/Ui/movementdialog.ui \
    src/Ui/spellsdialog.ui \
    src/Ui/roomeditattrdlg.ui
	
################################################ 	Main		######################################################
HEADERS += src/defines.h \ 
    src/Map/map.pb.h \
    src/mmapper/mmapper_importer.h \
    src/Gui/roomeditattrdlg.h
SOURCES += src/main.cpp \ 
    src/Map/map.pb.cc \
    src/mmapper/mmapper_importer.cpp \
    src/Gui/roomeditattrdlg.cpp

	
################################################ 	Engine		######################################################
HEADERS += src/Engine/CEngine.h \
    src/Engine/CCommandQueue.h \
    src/Engine/CEvent.h \
    src/Engine/CStacksManager.h 

SOURCES += src/Engine/CEngine.cpp \
    src/Engine/CStacksManager.cpp 
	
	
################################################ 	GroupManager		######################################################
HEADERS += src/GroupManager/CGroup.h \
    src/GroupManager/CGroupChar.h \
    src/GroupManager/CGroupClient.h \
    src/GroupManager/CGroupCommunicator.h \
    src/GroupManager/CGroupServer.h

SOURCES += src/GroupManager/CGroup.cpp \
    src/GroupManager/CGroupChar.cpp \
    src/GroupManager/CGroupClient.cpp \
    src/GroupManager/CGroupCommunicator.cpp \
    src/GroupManager/CGroupServer.cpp

	
################################################ 	Gui		######################################################
HEADERS += src/Gui/CActionManager.h \
    src/Gui/CLogDialog.h \
    src/Gui/CMovementDialog.h \
    src/Gui/ConfigWidget.h \
    src/Gui/CSelectionManager.h \
    src/Gui/finddialog.h \
    src/Gui/mainwindow.h \
    src/Gui/RoomEditDialog.h \
    src/Gui/SpellsDialog.h \
    src/Gui/CGroupSettingsDialog.h

SOURCES += src/Gui/CActionManager.cpp \
    src/Gui/CLogDialog.cpp \
    src/Gui/CMovementDialog.cpp \
    src/Gui/ConfigWidget.cpp \
    src/Gui/CSelectionManager.cpp \
    src/Gui/finddialog.cpp \
    src/Gui/mainwindow.cpp \
    src/Gui/SpellsDialog.cpp \
    src/Gui/CGroupSettingsDialog.cpp

	
################################################ 	Map		######################################################
HEADERS += src/Map/CRoom.h \
    src/Map/CRoomManager.h \
    src/Map/CTree.h \
    src/Map/CRegion.h

SOURCES += src/Map/CRoom.cpp \
    src/Map/CRoomManager.cpp \
    src/Map/CTree.cpp \
    src/Map/CRegion.cpp

	
################################################ 	Proxy		######################################################
HEADERS += src/Proxy/CDispatcher.h \
	src/Proxy/patterns.h \
    src/Proxy/proxy.h \
    src/Proxy/userfunc.h 
	

SOURCES += src/Proxy/CDispatcher.cpp \
	src/Proxy/patterns.cpp \
    src/Proxy/proxy.cpp \
    src/Proxy/userfunc.cpp 

	
################################################ 	Renderer	######################################################
HEADERS += src/Renderer/CFrustum.h \
    src/Renderer/CSquare.h \
    src/Renderer/renderer.h 


SOURCES += src/Renderer/CFrustum.cpp \
    src/Renderer/CSquare.cpp \
    src/Renderer/renderer.cpp 

	
################################################ 	Utils		######################################################
HEADERS += src/Utils/CTimers.h \
    src/Utils/CConfigurator.h \
    src/Utils/utils.h \
    src/Utils/xml2.h

SOURCES += src/Utils/CTimers.cpp \
    src/Utils/CConfigurator.cpp \
    src/Utils/utils.cpp \
    src/Utils/xml2.cpp
	
	
TARGET = pandoramapper

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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/protobuf/libs/ -llibprotobuf
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/protobuf/libs/ -llibprotobufd
else:unix: LIBS += -L$$PWD/protobuf/libs/ -llibprotobuf

INCLUDEPATH += $$PWD/protobuf/libs
DEPENDPATH += $$PWD/protobuf/libs

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/protobuf/libs/liblibprotobuf.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/protobuf/libs/liblibprotobufd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/protobuf/libs/libprotobuf.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/protobuf/libs/libprotobufd.lib
else:unix: PRE_TARGETDEPS += $$PWD/protobuf/libs/liblibprotobuf.a
