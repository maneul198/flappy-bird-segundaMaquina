#include"key.h"
#include<QObject>
#include<QCoreApplication>
#include<sys/reboot.h>
#include<QDebug>

int main(int argc, char *argv[]){

    QCoreApplication a(argc, argv);
    Key powerOff(2);
    QObject::connect(&powerOff, &Key::pressed, [=](){
        qDebug() << "POWEROFF" << endl;
        reboot(RB_POWER_OFF);

    });
    return a.exec();
}
