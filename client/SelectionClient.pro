QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    combat/katana.cpp \
    combat/weapon.cpp \
    combat/weaponmanager.cpp \
    entities/enemy.cpp \
    entities/entity.cpp \
    entities/player.cpp \
    game/game.cpp \
    map/map.cpp \
    map/tile.cpp \
    main.cpp \
    game/gameview.cpp \
    map/tilecollision.cpp \
    map/tilevisual.cpp \
    ui/mainwindow.cpp

HEADERS += \
    combat/katana.h \
    combat/weapon.h \
    combat/weaponmanager.h \
    entities/enemy.h \
    entities/entity.h \
    entities/player.h \
    game/game.h \
    game/gameview.h \
    map/map.h \
    map/tile.h \
    input/inputtypes.h \
    map/tilecollision.h \
    map/tilevisual.h \
    ui/mainwindow.h

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourses.qrc
