TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lboost_system -lpthread -lboost_filesystem

SOURCES += main.cpp

HEADERS += \
    read_server.h
