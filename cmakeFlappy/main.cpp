#include "mainwindow.h"
#include "awardPrize.h"
#include <QApplication>

//#define DEBUG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(0);
   //awardPrize w;

#ifdef DEBUG
    w.show();
#else
    w.showMaximized();
#endif

    return a.exec();
}
