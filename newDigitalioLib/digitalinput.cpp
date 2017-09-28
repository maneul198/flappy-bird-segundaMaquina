#include "digitalinput.h"
#include <QDebug>

void DPX::DigitalInput::internal_triggered(bool edge)
{
    /*
    if (m_bounceTimer) {
        if (m_bounceTimer->isActive())
            return;
            
        m_bounceTimer->start();
    }
    */
    
    emit triggered(edge);
}

void DPX::DigitalInput::init()
{
    if (m_bounceTimer) return;
    
    m_bounceTimer = new QTimer();
    m_bounceTimer->setInterval(m_bouncetime);
    m_bounceTimer->setSingleShot(true);
}

bool DPX::DigitalInput::read(unsigned int port, unsigned int pin)
{
    unsigned char value{0};
    int ret = dpci_io_read_port(port, &value);
    
    if (ret == -1)
        throw std::runtime_error(std::string("can't read digital input value on port: ") + std::to_string(port)
                                 + ", pin: " + std::to_string(pin));
                                 
    return (value & pin) != 0;
}

bool DPX::DigitalInput::read() const
{
    return read(m_port, m_pin);
}

int DPX::DigitalInput::readPort(unsigned int port)
{
    unsigned char value{0};
    int ret = dpci_io_read_port(port, &value);
    
    if (ret == -1)
        throw std::runtime_error(std::string("can't read digital input value on port: ") + std::to_string(port));
        
    return value;
}

int DPX::DigitalInput::readPort() const
{
    return readPort(m_port);
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
