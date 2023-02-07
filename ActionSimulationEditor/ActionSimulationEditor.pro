QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include($$PWD/../ActionSimulationBase/actionsimulationbase.pri)
include($$PWD/advanceddockingsystem/ads.pri)
include($$PWD/QXlsx/QXlsx.pri)

INCLUDEPATH += D:\Labraries/boost_1_80_0 \
               $$PWD/advanceddockingsystem/src \
               ../actionsimulationbase \

LIBS += -L$$PWD/advanceddockingsystem/lib

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    componentdialog.cpp \
    designcomponent.cpp \
    main.cpp \
    mainwindow.cpp \
    categorymanager.cpp \
    categorymodel.cpp \
    componentdelegate.cpp \
    componentmanager.cpp \
    componentmodel.cpp \
    frmcreateproject.cpp \
    frmtransferexcel.cpp \
    qautocompleteplaintextedit.cpp \
    qtsmalltools.cpp

HEADERS += \
    categorydelegate.h \
    categorymanager.h \
    categorymodel.h \
    componentdelegate.h \
    componentdialog.h \
    componentmanager.h \
    componentmodel.h \
    designcomponent.h \
    frmcreateproject.h \
    frmtransferexcel.h \
    mainwindow.h  \
    qautocompleteplaintextedit.h \
    qtsmalltools.h

FORMS += \
    componentdialog.ui \
    frmcreateproject.ui \
    frmtransferexcel.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = ActionSimulationEditor.ico

RESOURCES += \
    actionsimulationeditor.qrc
