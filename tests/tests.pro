TEMPLATE = app

CONFIG += qt testcase c++17
QT += testlib core

TARGET = pandora_tests

INCLUDEPATH = ../src ../src/Utils

SOURCES += \
    main.cpp \
    test_utils.cpp \
    test_room.cpp

HEADERS += \
    test_utils.h \
    test_room.h

# Include necessary source files from main project
SOURCES += \
    ../src/Utils/utils.cpp \
    ../src/Map/CRoom.cpp \
    ../src/Map/CTree.cpp \
    ../src/Map/CRegion.cpp

HEADERS += \
    ../src/Utils/utils.h \
    ../src/Map/CRoom.h \
    ../src/Map/CTree.h \
    ../src/Map/CRegion.h \
    ../src/defines.h

# Stubs for dependencies
DEFINES += TESTING_MODE
