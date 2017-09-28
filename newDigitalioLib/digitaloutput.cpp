#include "digitaloutput.h"

using namespace DPX;

DigitalOutput::DigitalOutput(const DPX::DigitalOutput &o)
    : QObject(o.parent()), m_port(o.m_port), m_pin(o.m_pin)
{
}

DPX::DigitalOutput &DigitalOutput::operator =(const DPX::DigitalOutput &o)
{
    setParent(o.parent());
    m_port = o.m_port;
    m_pin = o.m_pin;
    return *this;
}

int DigitalOutput::write(unsigned int port, unsigned int pin, bool value)
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_DOPorts[port][pin] = value;
    return dpci_io_write_port(port, m_DOPorts[port].to_ulong());
}

int DigitalOutput::write(bool value)
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_DOPorts[m_port][m_pin] = value;
    return dpci_io_write_port(m_port, m_DOPorts[m_port].to_ulong());
}

bool DigitalOutput::read() const
{
    return read(m_port, m_pin);
}

bool DigitalOutput::read(unsigned int port, unsigned int pin)
{
    unsigned char value{0};
    int ret = dpci_io_read_outport(port, &value);
    
    if (ret == -1)
        throw std::runtime_error(std::string("can't read digital output value on port: ") + std::to_string(port)
                                 + ", pin: " + std::to_string(pin));
                                 
    return (value & pin) != 0;
}

int DigitalOutput::readPort() const
{
    return readPort(m_port);
}

int DigitalOutput::readPort(unsigned int port)
{
    unsigned char value{0};
    int ret = dpci_io_read_outport(port, &value);
    
    if (ret == -1)
        throw std::runtime_error(std::string("can't read digital output value on port: ") + std::to_string(port));
        
    return value;
}

std::array<std::bitset<8>, 4> DigitalOutput::m_DOPorts;
std::mutex DigitalOutput::m_mutex;
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
