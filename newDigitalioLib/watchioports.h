#ifndef WATCHIOPORTS_H
#define WATCHIOPORTS_H

#include <QObject>
#include <QThread>
#include <QDebug>

#include <bitset>
#include <map>
#include <algorithm>
#include <atomic>

#include <dpci_core_api.h>

#include "digital_ports.h"
#include "digitalinput.h"

namespace DPX {
class WatchDIPorts;
}

class DPX::WatchDIPorts : public QThread {
    Q_OBJECT
    void run() override;
    
public:

    virtual ~WatchDIPorts() {}
    
    // singleton member
    static WatchDIPorts &self() {
        static WatchDIPorts wdi;
        return wdi;
    }
    
    bool isRegistered(unsigned int port, unsigned int pin) const;
    
    bool unregister(unsigned int port, unsigned int pin);
    bool unregister(const DPX::DigitalInput &di);
    
    DPX::DigitalInput *registerDigitalInput(unsigned int port
                                            , unsigned int pin
                                            , DPX::Edge edge = DPX::Both
                                            , unsigned int bouncetime = 0);
                                            
private:
    WatchDIPorts(QObject *parent = nullptr) : QThread(parent) {}
    
    unsigned long m_di {0};
    unsigned long m_edge {0};
    unsigned long m_autoconfigEdge {0};
    unsigned long m_mask {0};
    
    std::atomic<bool> m_update_di {false};
    std::map<DPX::Line, DPX::DigitalInput *> m_diMap;

signals:
    void vuele(bool v);
    
};


#endif // WATCHIOPORTS_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
