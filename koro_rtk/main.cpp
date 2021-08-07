#include "navimain.h"

#include <QApplication>

int main(int argc, char *argv[])
{


    QApplication a(argc, argv);
    NaviMain w;

    w.setWindowTitle("koro_rtk test");



    w.show();
    return a.exec();
}
