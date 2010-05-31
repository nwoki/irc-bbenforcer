# -------------------------------------------------
# Project created by QtCreator 2010-05-20T11:57:54
# -------------------------------------------------
QT += network \
      sql
QT -= gui
TARGET = ioQIC
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += src/main.cpp \
    src/Brain.cpp \
    src/IrcController.cpp \
    src/DbController.cpp \
    src/GameController.cpp \
    src/Connection.cpp
HEADERS += src/Brain.h \
    src/IrcController.h \
    src/DbController.h \
    src/GameController.h \
    src/Connection.h
OBJECTS_DIR = .obj
