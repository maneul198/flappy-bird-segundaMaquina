#ifndef KEY_H
#define KEY_H

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QSettings>

#ifdef GPIO
    #include <gpio.h>
    #include <gpio_types.h>
#else
    #include <digitalinput.h>
    #include <watchioports.h>
    #include <digital_ports.h>
#endif

class Key : public QObject {
    Q_OBJECT
    
public:
    Key(DPX::Line inputpin, uint bouncetime = 100, QObject *parent = 0);
    ~Key();
    int a;

signals:
    void pressed();
    void released();
    
private slots:
    void internal_pressed(bool edge);
    
private:

#ifdef GPIO
    GPIO *key {nullptr};
    bool m_noise {false};
    bool m_pressed {false};
#else
    DPX::DigitalInput *key {nullptr};
#endif
};

#endif // KEY_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
