#ifndef ID003_LIB_V3_H
#define ID003_LIB_V3_H

#include "id003_lib_v3_global.h"
#include <QObject>
#include <QThread>
#include <QDebug>
#include <Idcomm.h>
#include <QSettings>
//#include <logs/KbiredUtilities.h>

using namespace std;
class ID003_LIB_V3SHARED_EXPORT ID003_Lib_V3: public QThread  {
    Q_OBJECT
    void run() override;
public:
    ID003_Lib_V3();
    char z;
    char barcode[18];
public slots:
    bool abrirPuerto(char *dev);
    void apagar();
signals:
    void entroDinero(int dinero);
    void stackerFull();
    void stackerRetirado();
    void noteJammed();
    void billeteroPausado();
    void intentoFraude();
    // La señal fallaEnBilletero, representa una falla ficica irrecuperable, porfavor detenga la aplicacion y llame a servicio tecnico para remplazo
    void fallaEnBilletero(unsigned int falla_especifica);
    void errorEnComunicacion();
    void comandoInvalido();
    void estadoIndeterminado();
    void openPortError();
    void TITOIN();
private:
    //variables usadas por el hilo de comunicación con el IDE003 (billetero)
    int bc1=0, bc2=0;
    int Handle;
    char tempstrh[50];
    int denom;
    char billsEnable;
};

#endif // ID003_LIB_V3_H
