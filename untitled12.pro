TEMPLATE = app

QT += qml quick

CONFIG += c++17

SOURCES += main.cpp \
    widget.cpp \
    function.cpp \
    task.cpp

RESOURCES += qml.qrc

HEADERS += \
    widget.h \
    function.h \
    task.h

# Default rules for deployment.
include(deployment.pri)
