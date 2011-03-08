# -------------------------------------------------
# Project created by QtCreator 2010-05-20T11:57:54
# -------------------------------------------------
QT += network \
      sql

QT -= gui

TARGET = ioQIC-BBEnforcer

CONFIG += console \
          debug

CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/main.cpp \
    src/brain.cpp \
    src/irccontroller.cpp \
    src/dbcontroller.cpp \
    src/gamecontroller.cpp \
    src/ircuserscontainer.cpp

HEADERS += src/brain.h \
    src/irccontroller.h \
    src/dbcontroller.h \
    src/gamecontroller.h \
    src/ircuserscontainer.h

OBJECTS_DIR = .obj
