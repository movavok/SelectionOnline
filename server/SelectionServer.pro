TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        ai/enemy.cpp \
        main.cpp \
        server.cpp

HEADERS += \
    ai/enemy.h \
    server.h
