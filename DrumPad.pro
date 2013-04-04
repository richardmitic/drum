# Qt project file - qmake uses his to generate a Makefile

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DrumPad

LIBS += -lqwt -lm -lbcm2835 -lrt

HEADERS += window.h ad7705.h gz_clk.h

SOURCES += main.cpp window.cpp ad7705.cpp gz_clk.cpp
