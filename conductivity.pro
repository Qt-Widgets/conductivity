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
QT += widgets
QT += serialport


TARGET = conductivity
TEMPLATE = app

windows {
  # For National Instruments DAQ & GPIB Boards
  INCLUDEPATH += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include"
}

SOURCES += main.cpp
SOURCES += cdatastream2d.cpp
SOURCES += plot2d.cpp
SOURCES += mainwindow.cpp
SOURCES += utility.cpp
SOURCES += keithley236.cpp
SOURCES += axesdialog.cpp
SOURCES += AxisLimits.cpp
SOURCES += stripchart.cpp
SOURCES += AxisFrame.cpp
SOURCES += cdatastream.cpp
SOURCES += DataSetProperties.cpp
SOURCES += lakeshore330.cpp
SOURCES += configureIvsVdialog.cpp
SOURCES += configureRvsTdialog.cpp

HEADERS += mainwindow.h
HEADERS += cdatastream2d.h
HEADERS += plot2d.h
HEADERS += utility.h
HEADERS += keithley236.h
HEADERS += axesdialog.h
HEADERS += AxisLimits.h
HEADERS += stripchart.h
HEADERS += AxisFrame.h
HEADERS += cdatastream.h
HEADERS += DataSetProperties.h
HEADERS += lakeshore330.h
HEADERS += configureIvsVdialog.h
HEADERS += configureRvsTdialog.h

FORMS   += mainwindow.ui
FORMS   += configureIvsVdialog.ui
FORMS   += configureRvsTdialog.ui
FORMS   += axesdialog.ui

# For National Instruments GPIB Boards
win32 {
  message("Running on Windows 32 bit")
  LIBS += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib32/msvc/gpib-32.obj"
}
win64 {
  message("Running on Windows 64 bit")
  LIBS += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib32/msvc/gpib-32.obj"
}
linux {
  message("Running on Linux")
  LIBS += -L"/usr/local/lib" -lgpib # To include libgpib.so from /usr/local/lib
}

DISTFILES += \
    doc/linux_Gpib_HowTo.txt \
    doc/Keithley236Manual.pdf \
    doc/LakeShore330_Manual.pdf
