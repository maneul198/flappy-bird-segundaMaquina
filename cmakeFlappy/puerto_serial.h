#ifndef PUERTOSERIAL
#define PUERTOSERIAL

#include <QSerialPort>
#include <identificarArduino.h>

class puerto_serial : public QObject{
    Q_OBJECT

public:
    puerto_serial(QObject *parent = 0);

    void configure_puerto();

    bool write(const char *data);

    bool clear(QSerialPort::Direction direccion);

    QByteArray read(int max);

    int bytesAvailable();

    QSerialPort *puerto(){
        return &puertoSerial;
    }

private:
    static QSerialPort puertoSerial;

};








#endif
