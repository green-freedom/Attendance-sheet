#include "faceattendence.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // 分辨率自适应
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
       QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
       //finish
    QApplication a(argc, argv);
    FaceAttendence w;
    w.show();
    return a.exec();
}
