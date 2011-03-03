# -------------------------------------------------
# Project created by QtCreator 2010-05-20T11:57:54
# -------------------------------------------------
QT += network \
      sql

QT -= gui

TARGET = ioQIC-BBEnforcer

CONFIG += console

CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/main.cpp \
    src/brain.cpp \
    src/irccontroller.cpp \
    src/dbcontroller.cpp \
    src/gamecontroller.cpp \
    src/connection.cpp

HEADERS += src/brain.h \
    src/irccontroller.h \
    src/dbcontroller.h \
    src/gamecontroller.h \
    src/connection.h

OBJECTS_DIR = .obj
