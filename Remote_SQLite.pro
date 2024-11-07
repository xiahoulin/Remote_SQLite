QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    scriptwidget.cpp \
    sqlprocesshandler.cpp \
    tabledata.cpp \
    findtablewidget.cpp \
    socketmanager.cpp

HEADERS += \
    connectdialog.h \
    funcid.h \
    mainwindow.h \
    scriptwidget.h \
    sqlprocesshandler.h \
    tabledata.h \
    findtablewidget.h \
    socketmanager.h

FORMS += \
    connectdialog.ui \
    mainwindow.ui \
    scriptwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pics.qrc
