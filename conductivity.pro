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

# For National Instruments DAQ Boards
INCLUDEPATH += "C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev/include"
INCLUDEPATH += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include"

SOURCES += main.cpp \
    utility.cpp \
    keithley236.cpp \
    axesdialog.cpp \
    AxisLimits.cpp \
    stripchart.cpp \
    AxisFrame.cpp \
    cdatastream.cpp \
    DataSetProperties.cpp \
    lakeshore330.cpp \
    configuredialog.cpp
SOURCES += mainwindow.cpp

HEADERS += mainwindow.h \
    utility.h \
    keithley236.h \
    axesdialog.h \
    AxisLimits.h \
    stripchart.h \
    AxisFrame.h \
    cdatastream.h \
    DataSetProperties.h \
    lakeshore330.h \
    configuredialog.h

FORMS   += mainwindow.ui \
    axesdialog.ui \
    configuredialog.ui

# For National Instruments DAQ Boards
LIBS += "C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev/lib/msvc/NIDAQmx.lib"
LIBS += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib32/msvc/gpib-32.obj"
