QT       += widgets core gui

greaterThan(QT_MAJOR_VERSION, 4): {
    QT += widgets
    DEFINES +=QT5
}
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../RTKLib.pri)

TARGET = koro_rtk v1.0
TEMPLATE = app

INCLUDEPATH += ../src


#linux{
#    RTKLIB =../src/libRTKLib.a
#    LIBS += -lpng $${RTKLIB}
#}
#macx{
#    RTKLIB =../src/libRTKLib.a
#    LIBS += /usr/local/lib/libpng.a $${RTKLIB}
#}
win32 {
    RTKLIB =../src/libRTKLib.a
    LIBS+= $${RTKLIB} -lWs2_32 -lwinmm
}

PRE_TARGETDEPS = $${RTKLIB}


SOURCES += \
    main.cpp \
    navimain.cpp

HEADERS += \
    navimain.h

FORMS += \
    navimain.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
