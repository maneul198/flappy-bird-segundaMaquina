#ifndef DIGITALOUTPUT_H
#define DIGITALOUTPUT_H

#include "digital_ports.h"

#include <QObject>

#include <bitset>
#include <array>
#include <mutex>
#include <string>
#include <exception>
#include <dpci_core_api.h>

namespace DPX {
class DigitalOutput;
}

class DPX::DigitalOutput : public QObject {
    Q_OBJECT
public:
    explicit DigitalOutput(unsigned int port, unsigned int pin, QObject *parent = nullptr)
        : QObject(parent), m_port(port), m_pin(pin)
    {}
    
    DigitalOutput(const DigitalOutput &o);
    DigitalOutput &operator=(const DigitalOutput &o);
    
public slots:
    static int write(unsigned int port, unsigned int pin, bool value);
    int write(bool value);
    
    /**
     * @brief current port, range [0, 3]
     */
    inline int port() const {
        return m_port;
    }
    
    /**
     * @brief current pin, range [0, 7]
     */
    inline int pin() const {
        return m_pin;
    }
    
    /**
     * @brief read the last value on DigitalOutput pin
     */
    bool read() const;
    
    /**
     * @brief read the last value on DigitalOutput pin
     */
    static bool read(unsigned int port, unsigned int pin);
    
    /**
     * @brief read the last value on DigitalOutput port
     */
    int readPort() const;
    
    /**
     * @brief read the last value on DigitalOutput port
     */
    static int readPort(unsigned int port);
    
    /**
     * @brief line is the absolute pin e.g.
     * \code
     * for port: 1, pin: 2, is {b0000 0100 0000 0000} or 2^10 == 1024
     * for port: 0, pin: 7, is {b0000 0000 1000 0000} or 2^7 == 128
     * \endcode
     */
    inline DPX::Line line() const {
        return toLine(m_port, m_pin);
    }
    
private:
    int m_port {0};
    int m_pin {0};
    
    static std::mutex m_mutex;
    static std::array<std::bitset<8>, 4> m_DOPorts;
};

#endif // DIGITALOUTPUT_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
