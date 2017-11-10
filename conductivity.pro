#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T13:22:49
#
#-------------------------------------------------

QT += core
QT += gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = conductivity
TEMPLATE = app

# For National Instruments DAQ & GPIB Boards
INCLUDEPATH += "C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev/include"
INCLUDEPATH += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include"

SOURCES += main.cpp
SOURCES += utility.cpp
SOURCES += keithley236.cpp
SOURCES += axesdialog.cpp
SOURCES += AxisLimits.cpp
SOURCES += stripchart.cpp
SOURCES += AxisFrame.cpp
SOURCES += cdatastream.cpp
SOURCES += DataSetProperties.cpp
SOURCES += lakeshore330.cpp
SOURCES += configuredialog.cpp
SOURCES += mainwindow.cpp

HEADERS += mainwindow.h
HEADERS += utility.h
HEADERS += keithley236.h
HEADERS += axesdialog.h
HEADERS += AxisLimits.h
HEADERS += stripchart.h
HEADERS += AxisFrame.h
HEADERS += cdatastream.h
HEADERS += DataSetProperties.h
HEADERS += lakeshore330.h
HEADERS += configuredialog.h

FORMS   += mainwindow.ui
FORMS   += axesdialog.ui
FORMS   += configuredialog.ui

# For National Instruments DAQ & GPIB Boards
LIBS += "C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev/lib/msvc/NIDAQmx.lib"
LIBS += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib32/msvc/gpib-32.obj"
