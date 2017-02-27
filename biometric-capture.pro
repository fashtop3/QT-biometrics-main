#-------------------------------------------------
#
# Project created by QtCreator 2016-11-24T11:19:10
#
#-------------------------------------------------

QT       += core gui winextras xml webenginewidgets webchannel network
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG(debug, debug|release) {
    TARGET = ../../biometric-installer/packages/com.main.product/data/DDTFPBiometric
} else {
    TARGET = DDTFPBiometric
}

TEMPLATE = app


SOURCES += main.cpp \
    evdialog.cpp \
    fedialog.cpp \
    fvdialog.cpp \
    runguard.cpp \
    xmlreadwrite.cpp \
    gitprocessdialog.cpp \
    verifyworker.cpp \
    fpdatav.cpp \
    mainwindow.cpp \
    logindialog.cpp \
    webview.cpp \
    networkdata.cpp \
    webenginepage.cpp

HEADERS  += \
    evdialog.h \
    fedialog.h \
    fvdialog.h \
    fptpath.h \
    runguard.h \
    xmlreadwrite.h \
    gitprocessdialog.h \
    verifyworker.h \
    fpdatav.h \
    mainwindow.h \
    logindialog.h \
    webview.h \
    networkdata.h \
    webenginepage.h

FORMS += \
    evdialog.ui \
    fedialog.ui \
    fvdialog.ui \
    gitprocessdialog.ui \
    verificationdialog.ui \
    mainwindow.ui \
    logindialog.ui


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

RESOURCES += \
    resources.qrc

win32: LIBS += -lgdi32 -lCrypt32
