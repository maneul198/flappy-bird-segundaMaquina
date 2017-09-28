#ifndef DIGITALINPUT_H
#define DIGITALINPUT_H

#include "digital_ports.h"

#include <QObject>
#include <QTimer>
#include <QEvent>


#include <memory>
#include <exception>
#include <string>

#include <dpci_core_api.h>

namespace DPX {
class DigitalInput;
class WatchDIPorts;

enum Edge { Rising  = 1    ///> Low to high transition
, Falling = 0    ///> High to low transition
, Both    = 2    ///> High to low and low to high transition
          }; ///> Rising and Falling
}

class DPX::DigitalInput : public QObject {
    Q_OBJECT
    
    friend class WatchDIPorts;
    
public:
    DigitalInput(unsigned int port, unsigned int pin, DPX::Edge edge, unsigned int bouncetime = 100, QObject
                 * parent = nullptr)
        : QObject(parent), m_port(port), m_pin(pin), m_edge(edge), m_bouncetime(bouncetime)
    {}
    
    DigitalInput(const DigitalInput &) = delete;
    DigitalInput &operator=(const DigitalInput &) = delete;
    
    virtual ~DigitalInput() {
        if (m_bounceTimer) {
            m_bounceTimer->stop();
            delete m_bounceTimer;
        }
    }
    
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
     * @brief read the last value on DigitalInput pin
     */
    bool read() const;
    
    /**
     * @brief read the last value on DigitalInput pin
     */
    static bool read(unsigned int port, unsigned int pin);
    
    /**
     * @brief read the last value on DigitalInput port
     */
    int readPort() const;
    
    /**
     * @brief read the last value on DigitalInput port
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
    
    /**
     * @brief current edge cofigured
     */
    inline DPX::Edge edge() const {
        return m_edge;
    }
    
    /**
     * @brief bounce time minumum
     */
    inline bool bouncetime() const {
        return m_bouncetime;
    }
    
private:
    QTimer *m_bounceTimer {nullptr};
    void init();
    void internal_triggered(bool edge);
    
    const unsigned int m_port;
    const unsigned int m_pin;
    const DPX::Edge    m_edge;
    const unsigned int m_bouncetime;
    
signals:

    /**
     * @brief signal emitted when ocurr an DigitalInput event
     */
    void triggered(bool value);
};

#endif // DIGITALINPUT_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
