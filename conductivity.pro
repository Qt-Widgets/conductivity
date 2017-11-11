#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T13:22:49
#
#-------------------------------------------------

#Copyright (C) 2016  Gabriele Salvato

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.


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
