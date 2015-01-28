
# Folder containing updates to the Android package template
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp

RESOURCES += \
    qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    android/AndroidManifest.xml
