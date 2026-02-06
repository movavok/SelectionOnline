TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
QT += core network

SOURCES += \
        ai/enemy.cpp \
        main.cpp \
        server.cpp

HEADERS += \
    ai/enemy.h \
    server.h \
    shared/net/packet.h \
    shared/net/protocol.h
