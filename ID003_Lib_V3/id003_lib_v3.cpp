#include "id003_lib_v3.h"
#include "IDcomand.h"
#include "Idcomm.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include  <fcntl.h>
//#include <systemd/sd-journal.h>
#include <logs/KbiredUtilities.h>
ID003_Lib_V3::ID003_Lib_V3()
{
    QSettings settings("/home/" + qgetenv("USER") + "/.config/flappyBirdConfig/billetes.ini", QSettings::IniFormat);
    billsEnable= settings.value("billsEnable", 0).toInt();
}


bool ID003_Lib_V3::abrirPuerto(char *dev){
    int* ptr=&Handle;
    int respuesta=0;
    KbiredUtilities::logd("id003_state_machine: Opening: %s ",dev);
    KbiredUtilities::logd("id003_state_machine: Opening: %s ",dev);
    respuesta=Bill_OpenDev(dev,ptr);
    if ( respuesta == 0 ) {
        KbiredUtilities::logd("id003_state_machine: OpenDev fail");
        KbiredUtilities::logd("id003_state_machine: OpenDev fail");
        emit openPortError();
        //emit stateChanged( "-1" );
        return false;
    }else{
        return true;
    }
}

void ID003_Lib_V3::apagar()
{
    id003_inh_FF(Handle);
    sleep(0.1);
    Bill_CloseDev(Handle);
}




void ID003_Lib_V3::run() {
    KbiredUtilities::logd("id003_state_machine: Begin id003");
    int Rclear=0;
    int n, si = 0, j = 0;
    bool vend_valid=false;
    bool Buff_init_control=true;
    uint state = 0;
    char status[22];
    int valu = 0;
    unsigned int m=0;

    tcflush( Handle, TCIFLUSH );
    id003_Barkodeinh( Handle );
    n = ReadRespond( Handle, status, 7, 100000 );
    KbiredUtilities::logd("id003_state_machine: Respuesta del Barcode inh: %u",( ( ( unsigned int ) status[2] ) & 0x000000ff ));

    tcflush( Handle, TCIFLUSH );
    sleep( 0.1 );

    id003_Barkfun( Handle );
    n = ReadRespond( Handle, status, 7, 100000 );

    tcflush( Handle, TCIFLUSH );

    KbiredUtilities::logd("id003_state_machine: resp barcode config: %u ", ( ( ( unsigned int ) status[2] ) & 0x000000ff ));
    tcflush( Handle, TCIFLUSH );
    id003_Reqbari( Handle );
    n = ReadRespond( Handle, status, 6, 100000 );
    KbiredUtilities::logd("id003_state_machine: Validando estado: %u ",( ( ( int )status[3] ) & 0x000000ff ));
    id003_reset( Handle );
    sleep( 0.01 );
    forever {

        Rclear=-1;
        while (Rclear==-1) {
            usleep(200000);
            Rclear=tcflush(Handle, TCIOFLUSH);
        }

        id003_poll( Handle );

        n = ReadRespond( Handle, status, 22, 100000 );
        tcflush( Handle, TCIFLUSH );
        j=0;
        if ( n < 2 ) {
            for ( int i = 0; i < 22; i++ ){
                status[i] = 0;
            }
        }else{
            Buff_init_control=true;
            j=0;
            while (Buff_init_control) { // ciclo para controlar el inicio de trama.
                if((status[j]==(char)0xfc) and (status[j+1]==(char)0xfc)){ // Pregunta si el inicio de trama esta repetido.
                    j++; //Cuenta el numero de veces que el inicio de trama se repite de manera consecutiva.
                }else{
                    Buff_init_control=false; //Desactiva ciclo, puesto que ya se encontro el verdadero inicio de trama.
                    if(j>0){ //Condicional ara saber si hay corrimiento debido a inicio de trama repetido, encaso de no haber todo esta bien.
                        for ( int i = 0; i < (n-j); i++ ){
                            status[i] = status[i+j]; //Corrimiento del bufer para corregir posiciones predeterminadas.
                        }
                    }
                }
            }
        }
        KbiredUtilities::logd("id003 Bajo nivel: ---------------------Poll int");
        for ( int i = 0; i < (n-j); i++ ){
            KbiredUtilities::logd("id003 Bajo nivel: R[%u]: %u ",i,( ( ( int )status[i] ) & 0x000000ff ));
        }
        KbiredUtilities::logd("id003 Bajo nivel: ---------------------Poll Fin");
        sleep( 0.1 );
        si = ( ( ( unsigned int ) status[2] ) & 0x000000ff );

        switch ( si ) {
        case 18:
            KbiredUtilities::logd("id003_state_machine: Aceptando Billete");
            break;
        case 19:
            KbiredUtilities::logd("id003_state_machine: Identificando billete");
            //print ord(status[2])
            valu = ( ( ( unsigned int ) status[3] ) & 0x000000ff );

            if ( ( valu > 96 ) && ( valu < 103 ) ) {
                id003_stack_1( Handle );
                denom= dinfun( valu );
                vend_valid=false;
                KbiredUtilities::logd("id003_state_machine: dinero identificado: %d ", denom); // Dinero
            } else if ( valu == 111 ) {
                memset( tempstrh, 0x00, 50 );
                sprintf( tempstrh, "Barcode= " );
                for(int i = 0; i < 18; i++ )
                {
                    sprintf( tempstrh, "%u ",status[i+4]);
                    barcode[i] = status[i + 4];
                }
                sprintf( tempstrh, "%s\n", tempstrh);
                id003_hold(Handle);
                tcflush(Handle,TCIFLUSH);
                KbiredUtilities::logd(" id003_state_machine: barcode %s . \n",tempstrh);
                z=0;
                emit TITOIN();
                while(z==0){ //Sostiene el TITO hasta que el ususario oprime el boton de acceptar o rechazar.
                    sleep(0.01);
                    id003_hold(Handle);
                }
                if (z==1){
                    id003_stack_1(Handle);
                    tcflush(Handle,TCIFLUSH);
                    KbiredUtilities::logd(" IPRO-RC_state_Machine: TITO almacenado. \n");
                    //----------------
                    //mencont=17;
                    //----------------

                }else{
                    id003_retorno(Handle);
                    tcflush(Handle,TCIFLUSH);
                    KbiredUtilities::logd(" IPRO-RC_state_Machine: TITO Rechazado. \n");
                    //----------------
                    //mencont=18;
                    //----------------

                }
                z=0;
            }
            break;
        case 20:
            KbiredUtilities::logd("id003_state_machine: Almacenando en stacker");
            break;
        case 21:
            state = ( ( ( unsigned int ) ( status[2] ) ) & 0x000000ff );
            id003_Ack( Handle );
            if(vend_valid==false){
                vend_valid=true;
                emit entroDinero(denom);
            }
            KbiredUtilities::logd("id003_state_machine: venvalid: %u ", state);
            break;
        case 22:
            KbiredUtilities::logd("id003_state_machine: Billete en stacker");
            break;
        case 23:
            state = ( ( ( unsigned int ) status[3] ) & 0x000000ff );
            KbiredUtilities::logd("id003_state_machine: Motivo Rechazo: %u ", state);
            break;
        case 24:
            KbiredUtilities::logd("id003_state_machine: Regresando Billete por Orden");
            break;
        case 25:
            KbiredUtilities::logd("id003_state_machine: Reteniendo Billete");
            break;
        case 26:
            KbiredUtilities::logd("id003_state_machine: Deshabilitado.");
            id003_inh( Handle );
            break;
        case 27:
            KbiredUtilities::logd("id003_state_machine: Inicializando.");
            tcflush( Handle, TCIFLUSH );
            id003_enable2( Handle, billsEnable );
            n = ReadRespond( Handle, status, 5, 100000 );
            tcflush( Handle, TCIFLUSH );
            id003_seguridad( Handle );
            n = ReadRespond( Handle, status, 5, 100000 );
            tcflush( Handle, TCIFLUSH );
            id003_funcionop( Handle );
            n = ReadRespond( Handle, status, 5, 100000 );
            tcflush( Handle, TCIFLUSH );
            id003_inh( Handle );

            forever {
                id003_poll( Handle );
                n = ReadRespond( Handle, status, 5, 1000000 );
                tcflush( Handle, TCIFLUSH );
                if ( ( unsigned int ) status[2] == 17 ){
                    break;
                }else if(( unsigned int ) status[2] == 26 ){
                    id003_inh( Handle );
                    tcflush( Handle, TCIFLUSH );
                }
            }

            KbiredUtilities::logd("id003_state_machine: Configurando Barcode");

            sleep( 0.1 );
            id003_Barkodeinh( Handle );
            n = ReadRespond( Handle, status, 7, 100000 );

            KbiredUtilities::logd("id003_state_machine: resp barcode inh: %u", ( ( ( unsigned int ) status[2] ) & 0x000000ff ));
            tcflush( Handle, TCIFLUSH );
            sleep( 0.1 );
            id003_Barkfun( Handle );

            n = ReadRespond( Handle, status, 7, 100000 );

            tcflush( Handle, TCIFLUSH );
            KbiredUtilities::logd("id003_state_machine: resp barcode config: %u Barcode configurado",( ( ( unsigned int ) status[2] ) & 0x000000ff ));
            break;
        case 64:
            tcflush(Handle,TCIFLUSH);
            KbiredUtilities::logd("id003_state_machine:  Power Up \n");
            id003_reset(Handle);
            n=ReadRespond(Handle, status, 5,100000);
            tcflush(Handle,TCIFLUSH);
            m=(((unsigned int) (status[2]))&0x000000ff);
            if(m==0x50){
                KbiredUtilities::logd("id003_state_machine:  IPRO-RC_state_Machine: Reset ACK. \n");
            }else{
                KbiredUtilities::logd("id003_state_machine: Reset invalido. \n");
            }
            break;
        case 65:
            tcflush(Handle,TCIFLUSH);
            KbiredUtilities::logd("id003_state_machine:  Power Up whit bill in Aceptor\n");
            id003_reset(Handle);
            n=ReadRespond(Handle, status, 5,100000);
            tcflush(Handle,TCIFLUSH);
            m=(((unsigned int) (status[2]))&0x000000ff);
            if(m==0x50){
                KbiredUtilities::logd("id003_state_machine:  IPRO-RC_state_Machine: Reset ACK. \n");
            }else{
                KbiredUtilities::logd("id003_state_machine: Reset invalido. \n");
            }
            break;
        case 66:
            tcflush(Handle,TCIFLUSH);
            KbiredUtilities::logd("id003_state_machine:  Power Up whit bill in stacker\n");
            id003_reset(Handle);
            n=ReadRespond(Handle, status, 5,100000);
            tcflush(Handle,TCIFLUSH);
            m=(((unsigned int) (status[2]))&0x000000ff);
            if(m==0x50){
                KbiredUtilities::logd("id003_state_machine:  IPRO-RC_state_Machine: Reset ACK. \n");
            }else{
                KbiredUtilities::logd("id003_state_machine: Reset invalido. \n");
            }
            break;
        case 67:
            KbiredUtilities::logd("id003_state_machine: Error--> full");
            emit stackerFull();
            break;
        case 68:
            KbiredUtilities::logd("id003_state_machine: Error--> Open");
            emit stackerRetirado();
            id003_reset( Handle );
            break;
        case 69:
            KbiredUtilities::logd("id003_state_machine: Error--> jamA"); // Nota atrancada
            emit noteJammed();
            id003_reset( Handle );
            break;
        case 70:
            KbiredUtilities::logd("id003_state_machine: Error--> jamS"); // Nota atrancada
            emit noteJammed();
            id003_reset( Handle );
            break;
        case 71:
            KbiredUtilities::logd("id003_state_machine: Error--> pause");
            emit billeteroPausado();
            //id003_reset( Handle );
            break;
        case 72:
            KbiredUtilities::logd("id003_state_machine: Error--> cheated");
            emit intentoFraude();
            id003_reset( Handle );
            break;
        case 73:
            KbiredUtilities::logd("id003_state_machine: Error--> failure");
            m=(((unsigned int) (status[3]))&0x000000ff);
            emit fallaEnBilletero(m);
            if(m==0xA2){
                KbiredUtilities::logd("id003_state_machine: Falla en el Motor del Stacker");
            }else if(m==0xA5){
                KbiredUtilities::logd("id003_state_machine: Falla en la velocidad del motor de alimentacion");
            }else if(m==0xA6){
                KbiredUtilities::logd("id003_state_machine: Falla en el motor de alimentacion");
            }else if(m==0xAB){
                KbiredUtilities::logd("id003_state_machine: Cashbox not ready");
            }else if(m==0xAF){
                KbiredUtilities::logd("id003_state_machine: Cabeza del validador ausento o tipo erroneo instalado");
            }else if(m==0xB0){
                KbiredUtilities::logd("id003_state_machine: Falla de booteo en la ROM");
            }else if(m==0xB1){
                KbiredUtilities::logd("id003_state_machine: Falla de la ROM Externa");
            }else if(m==0xB2){
                KbiredUtilities::logd("id003_state_machine: Falla de la ROM");
            }else if(m==0xB3){
                KbiredUtilities::logd("id003_state_machine: Falla en la escritura de la ROM externa");
            }else{
                KbiredUtilities::logd("id003_state_machine: Error desconocido, posible falla de comunicacion");
            }
            //id003_reset( Handle );
            break;
        case 74:
            KbiredUtilities::logd("id003_state_machine: Error--> communication");
            emit errorEnComunicacion();
            id003_reset( Handle );
            break;
        case 75:
            KbiredUtilities::logd("id003_state_machine: Error--> invalid command");
            emit comandoInvalido();
            break;
        case 222:
            KbiredUtilities::logd("id003_state_machine: Pragram Signature Bussy");
            break;
        case 223:
            KbiredUtilities::logd("id003_state_machine: Pragram Signature END");
            break;
        default:
            if (si != 17){
                emit estadoIndeterminado();
                KbiredUtilities::logd("id003_state_machine: estado indeterminado %u ",( ( ( unsigned int ) status[2] ) & 0x000000ff ));
            }
            break;
        }

        si = 0;
    }

    id003_inh_FF( Handle );
    Bill_CloseDev( Handle );
    //emit stateChanged( "0" );
}
