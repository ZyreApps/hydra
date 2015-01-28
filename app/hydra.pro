
TARGET = hydra_app

# Folder containing updates to the Android package template
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp

RESOURCES += \
    qml/qml.qrc

# URIs of plugins with shared libraries to bundle on android
uri_QmlHydra = QmlHydra

ANDROID_EXTRA_LIBS = \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libsodium.so \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libzmq.so \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libczmq.so \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libzyre.so \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libhydra.so \
  $$[QT_INSTALL_QML]/$$replace(uri_QmlHydra,  \\., /)/libqml_hydra.so

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    android/AndroidManifest.xml
