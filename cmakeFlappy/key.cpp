#include "key.h"

Key::Key(DPX::Line inputpin, uint bouncetime, QObject *parent)
    : QObject(parent)
{
    QSettings config;
#ifdef GPIO
    key = new GPIO(BCM_pin);
    connect(key, SIGNAL(valChanged(string &)), this, SLOT(internal_pressed(string &)));
    
    key->export_gpio();
    key->setdir_gpio(GPIO::Direction::IN);
#else
    using DPX::WatchDIPorts;
    using DPX::DigitalInput;
    using DPX::DI;
    using DPX::GET_PORT;
    using DPX::GET_PIN;
    
    unsigned int port = DI[inputpin][GET_PORT];
    unsigned int pin = DI[inputpin][GET_PIN];

    auto &wdip = WatchDIPorts::self();

    key = wdip.registerDigitalInput(
              port,
              pin,
              DPX::Falling,
              bouncetime
          );
    a= 1;

    //connect(key, &DigitalInput::triggered, this, &Key::internal_pressed);
    connect(key, SIGNAL(triggered(bool)), this, SLOT(internal_pressed(bool)));

#endif
}

Key::~Key()
{
#ifdef GPIO
    key->unexport_gpio();
    delete key;
#else
    DPX::WatchDIPorts::self().unregister(*key);
#endif
}

//! SLOTS
void Key::internal_pressed(bool edge)
{
#ifdef GPIO

    if (state == GPIO::VAL_1) {
        emit pressed();
        m_pressed = true;
    } else if (m_pressed) {
        emit released();
        m_pressed = false;
    }
    
#else

    emit pressed();
}

#endif
 //!
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
