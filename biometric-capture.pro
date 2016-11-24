#-------------------------------------------------
#
# Project created by QtCreator 2016-11-24T11:19:10
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = biometric-capture
TEMPLATE = app


SOURCES += main.cpp \
    evdialog.cpp \
    fedialog.cpp \
    fvdialog.cpp

HEADERS  += \
    evdialog.h \
    fedialog.h \
    fvdialog.h

FORMS += \
    evdialog.ui \
    fedialog.ui \
    fvdialog.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -lDPFPApi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -lDPFPApi
else:unix: LIBS += -L$$PWD/../build-biometric-capture/ -lDPFPApi

INCLUDEPATH += $$PWD/../build-biometric-capture
DEPENDPATH += $$PWD/../build-biometric-capture

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -lDPFpUI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -lDPFpUI
else:unix: LIBS += -L$$PWD/../build-biometric-capture/ -lDPFpUI

INCLUDEPATH += $$PWD/../build-biometric-capture
DEPENDPATH += $$PWD/../build-biometric-capture

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -ldpHFtrEx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -ldpHFtrEx
else:unix: LIBS += -L$$PWD/../build-biometric-capture/ -ldpHFtrEx

INCLUDEPATH += $$PWD/../build-biometric-capture
DEPENDPATH += $$PWD/../build-biometric-capture

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -ldpHMatch
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-biometric-capture/ -ldpHMatch
else:unix: LIBS += -L$$PWD/../build-biometric-capture/ -ldpHMatch

INCLUDEPATH += $$PWD/../build-biometric-capture
DEPENDPATH += $$PWD/../build-biometric-capture