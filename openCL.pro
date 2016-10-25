#-------------------------------------------------
#
# Project created by QtCreator 2016-10-23T23:37:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = openCL
TEMPLATE = app

LIBS+= -lOpenCL -L$$PWD/ -L"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v8.0\lib\x64"
#LIBS+= -lOpenCL -L$$PWD/ -L"C:\Program Files (x86)\Intel\OpenCL SDK\6.1\lib\x64"

INCLUDEPATH += "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v8.0\include\\"
#INCLUDEPATH += "C:\Program Files (x86)\Intel\OpenCL SDK\6.1\include\\"

SOURCES += main.cpp\
        openCLWindow.cpp

HEADERS  += openCLWindow.h

FORMS    += openCLWindow.ui

DISTFILES += \
    kernel.cl
