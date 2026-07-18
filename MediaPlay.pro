QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    PacketQueue.cpp \
    audio_read.cpp \
    livedialog.cpp \
    logindialog.cpp \
    main.cpp \
    movielable.cpp \
    onlinedialog.cpp \
    picinpic_read.cpp \
    picturewidget.cpp \
    playerdialog.cpp \
    recorderdialog.cpp \
    savevideofilethread.cpp \
    uploaddialog.cpp \
    videoplayer.cpp

HEADERS += \
    PacketQueue.h \
    audio_read.h \
    common.h \
    livedialog.h \
    logindialog.h \
    movielable.h \
    onlinedialog.h \
    picinpic_read.h \
    picturewidget.h \
    playerdialog.h \
    recorderdialog.h \
    savevideofilethread.h \
    uploaddialog.h \
    videoplayer.h

FORMS += \
    livedialog.ui \
    logindialog.ui \
    movielable.ui \
    onlinedialog.ui \
    picturewidget.ui \
    playerdialog.ui \
    recorderdialog.ui \
    uploaddialog.ui

include(./opengl/opengl.pri)

INCLUDEPATH += ./opengl/

include(./netapi/netapi.pri)
INCLUDEPATH +=./netapi/

INCLUDEPATH += $$PWD/ffmpeg-4.2.2/include\
                $$PWD/SDL2-2.0.10/include\
                D:/BaiduNetdiskDownload/opencv-release/include/opencv2 \
                D:/BaiduNetdiskDownload/opencv-release/include

LIBS += $$PWD/ffmpeg-4.2.2/lib/avcodec.lib\
        $$PWD/ffmpeg-4.2.2/lib/avdevice.lib\
        $$PWD/ffmpeg-4.2.2/lib/avfilter.lib\
        $$PWD/ffmpeg-4.2.2/lib/avformat.lib\
        $$PWD/ffmpeg-4.2.2/lib/avutil.lib\
        $$PWD/ffmpeg-4.2.2/lib/postproc.lib\
        $$PWD/ffmpeg-4.2.2/lib/swresample.lib\
        $$PWD/ffmpeg-4.2.2/lib/swscale.lib\
        $$PWD/SDL2-2.0.10/lib/x86/SDL2.lib

LIBS += D:/BaiduNetdiskDownload/opencv-release/lib/libopencv_*.dll.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
