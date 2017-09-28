#ifndef CONNECT_H
#define CONNECT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

class Connect
{

public:
    Connect(){
            arduino_port_name = "";
            arduino_is_available = false;
    }

    QString nombre_puerto(){
            foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
                if (//serialPortInfo.description() == Descripcion &&
                        serialPortInfo.vendorIdentifier()== arduino_MEGA_vendedor_id
                        && serialPortInfo.productIdentifier()== arduino_MEGA_producto_id){
                    arduino_is_available = true;
                    arduino_port_name = serialPortInfo.portName();
                    qDebug() << "ARDUINO en " << serialPortInfo.portName().toLatin1();
                    break;

                } 
                qDebug() << ".";

            }


            if (arduino_is_available){
                qDebug() << "El arduino esta conectado y listo para usarse ";
            }else{

                qDebug() << "Ningun ARDUINO conectado";
            }
            return arduino_port_name;
        }


private:
    static const quint16 arduino_MEGA_vendedor_id = 9025;
    static const quint16 arduino_MEGA_producto_id = 66;
    QString arduino_port_name;
    QString Descripcion = "Arduino Mega 2560";
    bool arduino_is_available;




};

#endif // CONNECT_H
