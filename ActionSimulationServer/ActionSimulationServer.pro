QT += core

CONFIG += c++17 console
CONFIG -= app_bundle

include($$PWD/../ActionSimulationBase/actionsimulationbase.pri)
include($$PWD/qtservice/src/qtservice.pri)

INCLUDEPATH += D:/Labraries/boost_1_80_0  \
               D:/Labraries//LuaJIT-2.1.0-beta3/src  \
               ..\ActionSimulationBase \

LIBS += D:/Labraries/LuaJIT-2.1.0-beta3/src/lua51.lib  \

QMAKE_LFLAGS   = /NODEFAULTLIB:libcmt.lib

DEFINES += _WIN32_WINNT=0x0601

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        actionscript.cpp \
        actionsimulationserver.cpp \
        appconfig.cpp \
        boardcast.cpp \
        corecomponent.cpp \
        inputcomponent.cpp \
        main.cpp \
        normalcomponent.cpp \
        projectmanager.cpp \
        scheduledtaskpool.cpp \
        teammastercomponent.cpp \
        teamslavecomponent.cpp \
        threadpool.cpp \
        usermanager.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = ActionSimulationServer.ico

HEADERS += \
    actionscript.h \
    actionsimulationserver.h \
    appconfig.h \
    boardcast.h \
    corecomponent.h \
    inputcomponent.h \
    normalcomponent.h \
    projectmanager.h \
    scheduledtaskpool.h \
    teammastercomponent.h \
    teamslavecomponent.h \
    threadpool.h \
    usermanager.h
