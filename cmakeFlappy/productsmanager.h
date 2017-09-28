#ifndef PRODUCTSMANAGER_H
#define PRODUCTSMANAGER_H

#include <string>
#include <QObject>
#include <QList>
#include <atomic>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QSettings>
#include <QThread>
#include <QSound>
#include <QDebug>
#include <QTimer>
#include <QSerialPort>
#include <sstream>
#include <puerto_serial.h>

#ifdef GPIO
    #include <gpio.h>
#else
    //#include <digital_ports.h>
    //#include <watchioports.h>
    //#include <digitalinput.h>
#endif

#include "product.h"

class ProductsManager : public QObject, public QQmlParserStatus {
    Q_OBJECT

public:
    ProductsManager(QObject *parent = 0);

    void addProduct(Product *);
    
    Product *product(int index);
    
    void classBegin() override;
    void componentComplete() override;
    
    
    bool busy() const;
    
    bool enabledLights() const;
    void setEnabledLights(bool enable);
    
    int selected() const;
    
    int spinTime() const;
    void setSpinTime(int spinTime);
    
    int countHooks() const;

public slots:
    Q_INVOKABLE void turnHook(uint hook);
    Q_INVOKABLE void selectHook(uint hook);
    Q_INVOKABLE void unselectHooks();
    
signals:
    void delivered(Product *product);
    void deliveredNumberHook(uint hook);
    void started();
    void timeout(uint hook);
    void noProduct();
    void stopped();
    
    void productsChanged();
    void enabledLightsChanged();
    void selectedChanged();
    void busyChanged();
    void spinTimeChanged();
    
private:
    bool flagHook; //Bandera que indica los ganchos se encuentra en disponibles o en proseso de una entrega anterior y se esta esperando respuesta. Esta bandera se usara para bloquear el envio peticiones de entrega adicional al arduino hasta recibir respuesta.
    int ab;
    void turnHook();
    void stopHook();
    void setBusy(bool busy);
    void sensorChanged(bool value);
    /* Sound */
    QSound *soundfiesta;
    //EngineSV *m_engineHook {nullptr};
    //DPX::DigitalInput *m_sensor {nullptr};
    
    std::atomic<bool> m_busy;
    
    // hook selected
    uint m_currentHook {0};
    int m_turnOnCounter {3};
    int m_spinTime {2400};
    
    bool m_enableLights {false};
    bool m_delivered {false};
    int orden[10]= {9, 0, 8, 1, 7, 6, 5, 4, 3, 2};
    QTimer *timer;
    
    QList<Product *> m_products;
    Product m_none;
    Product* actual;
    puerto_serial serialPort;
};
#endif // PRODUCTSMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
