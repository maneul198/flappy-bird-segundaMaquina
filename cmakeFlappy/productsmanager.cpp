#include "productsmanager.h"
#include "gameelement.h"

ProductsManager::ProductsManager(QObject *parent) :
    QObject(parent)
{


#ifdef GPIO
    m_sensor = new GPIO(sensorGPIO.toString().toStdString());
    m_sensor->export_gpio();
    m_sensor->setdir_gpio(GPIO::Direction::IN);
#else

    /*
    connect(m_engineHook, &EngineSV::engineFinished, [&](){
        //m_spinTime= 800;
        QTimer::singleShot(800, this, [=]{
            turnHook();

        });
    });
    */

    /* Sound */
    soundfiesta = new QSound( GameElement::url + "sounds/fiesta.wav");


    timer= new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);

    serialPort.configure_puerto();
    flagHook=true;

    connect(serialPort.puerto(), &QSerialPort::readyRead, this, [&](){
        if(serialPort.bytesAvailable() >= 3){
            QString a(serialPort.read(1));
            qDebug() << a << endl;

            qDebug() << "PRUEBA" << endl;

            if(a == "T"){
                actual->decreaseCount();
                flagHook=true;
                serialPort.write("W1\r\n");
                soundfiesta->play();
                emit delivered(new Product());
                emit deliveredNumberHook(m_currentHook);
                if( m_currentHook == 9 || m_currentHook == 8
                        || m_currentHook == 0 || m_currentHook == 1)
                            serialPort.write("C3\r\n");
                else
                    serialPort.write("C2\r\n");


            }else if(a == "F"){
                flagHook=true;
            }
            serialPort.clear(QSerialPort::Input);
        }

        //serialPort->clear();

    });

#endif
}






Product *ProductsManager::product(int index)
{
    return m_products.at(index);
}

void ProductsManager::classBegin()
{
}

void ProductsManager::componentComplete()
{
}



bool ProductsManager::busy() const
{
    return m_busy.load();
}

bool ProductsManager::enabledLights() const
{
    return m_enableLights;
}

void ProductsManager::setEnabledLights(bool enable)
{
    m_enableLights = enable;
    emit enabledLightsChanged();
}

int ProductsManager::selected() const
{
    return m_currentHook;
}

int ProductsManager::spinTime() const
{
    return m_spinTime;
}

void ProductsManager::setSpinTime(int spinTime)
{
    m_spinTime = spinTime;
    emit spinTimeChanged();
}

int ProductsManager::countHooks() const
{
    return m_products.count();
}

void ProductsManager::turnHook(uint hook)
{
    m_currentHook = hook;

    actual=product(m_currentHook);
    int existencias=0;
    existencias=actual->count();
//    existencias=countHooks();
//    QString msn= QString("Existencias: %1").arg(existencias);
//    qDebug() << msn << endl;
    if(existencias>0){
        turnHook();
    }else{
        emit noProduct();
    }



}

void ProductsManager::turnHook()
{
    if(flagHook){
        flagHook=false;
        std::ostringstream oss;
        oss.str(""),

        oss << 'M' << m_currentHook << "\r\n";

        qDebug() << oss.str().c_str() << endl;

        serialPort.write(oss.str().c_str());
    }

}

void ProductsManager::stopHook()
{

}

void ProductsManager::setBusy(bool busy)
{
}

void ProductsManager::sensorChanged(bool value)
{

}


void ProductsManager::selectHook(uint hook)
{

}

void ProductsManager::unselectHooks()
{

}

void ProductsManager::addProduct(Product *product){
    m_products.append(product);

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
