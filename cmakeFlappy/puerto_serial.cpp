#include<puerto_serial.h>
#include <identificarArduino.h>

//QSerialPort puerto_serial::puertoSerial = NULL;

 puerto_serial::puerto_serial(QObject *parent)
     : QObject(parent)
 {

 }

void  puerto_serial::configure_puerto(){

     puertoSerial.setBaudRate(9600);
     puertoSerial.open(QIODevice::ReadWrite);
}

bool puerto_serial::write(const char *data)
{
    puertoSerial.write(data);

}

bool puerto_serial::clear(QSerialPort::Direction direcction){
    return puertoSerial.clear(direcction);
}

QByteArray puerto_serial::read(int max){
    return puertoSerial.read(max);

}

int puerto_serial::bytesAvailable(){
    return puertoSerial.bytesAvailable();
}


Connect con;
//QSerialPort puerto_serial::puertoSerial("/dev/ttyACM0");
QSerialPort puerto_serial::puertoSerial(con.nombre_puerto());

