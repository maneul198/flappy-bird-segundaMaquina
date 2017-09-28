#include "watchioports.h"

#include <QAbstractEventDispatcher>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QTime>

#include <QStringBuilder>

/*
DPX::DigitalInput *DPX::WatchDIPorts::registerDigitalInput(unsigned int port, unsigned int pin
        ,DPX::Edge edge, unsigned int bouncetime)
{
    //Line line = toLine(port, pin);

    Line line= pin;
    
    if (m_diMap.find(line) != m_diMap.end())
        return m_diMap[line];
        
    m_di |= line;
    
    if (edge == DPX::Both)
        m_autoconfigEdge |= line;
    else if (edge == DPX::Rising)
        m_edge |= line;
    else if (edge == DPX::Falling && (edge & line) != 0)
        m_edge ^= line;
        
    auto di_ptr = new DPX::DigitalInput(port, pin, edge, bouncetime, this);
    m_diMap[line] = di_ptr;
    
    m_update_di.store(true, std::memory_order_relaxed);
    start();
    return di_ptr;
}
*/


DPX::DigitalInput *DPX::WatchDIPorts::registerDigitalInput(unsigned int port, unsigned int pin
            ,DPX::Edge edge, unsigned int bouncetime){

    Line line= pin;
    auto di_ptr = new DPX::DigitalInput(port, pin, edge, bouncetime, this);
    m_diMap[line] = di_ptr;
    m_mask+=  pin;
    start();
    return di_ptr;
}

bool DPX::WatchDIPorts::unregister(unsigned int port, unsigned int pin)
{
    if (port >= DPX::DIGITAL_IO_PORTS || pin > 7)
        return false;
        
    Line line = toLine(port, pin);
    
    if (m_diMap.find(line) != m_diMap.end()) {
        if ((m_di & line) != 0)
            m_di ^= line;
            
        if ((m_autoconfigEdge & line) != 0)
            m_autoconfigEdge ^= line;
            
        if ((m_edge & line) != 0)
            m_edge ^= line;
            
        auto digitalInput = m_diMap[line];
        
        if (digitalInput) {
            delete digitalInput;
            digitalInput = nullptr;
        }
        
        m_diMap.erase(line);
        
        m_update_di.store(true);
        return true;
    }
    
    return false;
}

bool DPX::WatchDIPorts::unregister(const DPX::DigitalInput &di)
{

    return unregister(di.port(), di.pin());
}

void DPX::WatchDIPorts::run()
{
    constexpr uint TIMEOUT{500};
    std::unique_ptr<struct dio_event> dioData { new dio_event };
    //unsigned long lastDiConf = 0;
    qDebug() << "starting digital input watcher";
    int a= 1;

    forever {

        int ret= dpci_io_wait_int(m_mask, 0, TIMEOUT);
                                  if(ret > 0){
                                      DigitalInput *digitalInput = m_diMap[ret];
                                      //if(digitalInput)
                                      qDebug() << "KEY: " << a++ << " " << QTime::currentTime().msec() << endl;
                                      digitalInput->internal_triggered(true);
                                      msleep(80);
                                  }
    }
}


// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
