QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
 # Windows平台下的OpenCV和SeetaFace环境配置
win32{
    LIBS += E:\face\gsau\opencv452\x64\mingw\lib\libopencv*
    LIBS += E:\face\gsau\SeetaFace\lib\libSeeta*
    INCLUDEPATH += E:\face\gsau\opencv452\include
    INCLUDEPATH += E:\face\gsau\opencv452\include\opencv2
    INCLUDEPATH += E:\face\gsau\SeetaFace\include
    INCLUDEPATH += E:\face\gsau\SeetaFace\include\seeta
}
 #linux平台下的OpenCV和SeetaFace环境配置
unix{
    LIBS += -L/opt/opencv4-pc/lib -lopencv_world\
    -lSeetaFaceDetector\      # 人脸检测库
    -lSeetaFaceLandmarker\    # 人脸关键点检测库
    -lSeetaFaceRecognizer\    # 人脸识别库
    -lSeetaFaceTracker\       # 人脸跟踪库
    -lSeetaNet\               # SeetaNet深度学习框架
    -lSeetaQualityAssessor\   # 人脸质量评估库
    INCLUDEPATH += /opt/opencv4-pc/include/opencv4
    INCLUDEPATH += /opt/opencv4-pc/include/opencv4/opencv2
    INCLUDEPATH += /opt/opencv4-pc/include
    INCLUDEPATH += /opt/opencv4-pc/include/seeta
}
SOURCES += \
    main.cpp \
    attendancewin.cpp \
    qfaceobject.cpp \
    registerwin.cpp \
    selectwin.cpp

HEADERS += \
    attendancewin.h \
    qfaceobject.h \
    registerwin.h \
    selectwin.h

FORMS += \
    attendancewin.ui \
    registerwin.ui \
    selectwin.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
