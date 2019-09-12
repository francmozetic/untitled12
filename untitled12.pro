TEMPLATE = app

QT += qml quick

CONFIG += c++17

SOURCES += main.cpp \
    widget.cpp

RESOURCES += qml.qrc

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    widget.h
