/*
 * File:   idcomm.h
 * Author: cabitech
 *
 * Created on 25 de noviembre de 2015, 04:41 PM
 */

#ifndef IDCOMM_H
#define	IDCOMM_H


#ifdef	__cplusplus
extern "C" {
#endif

//#define DEBUG
#define PROC_EXIT           -1

#define BILL_CMD_POWERUP    200
#define BILL_CMD_GETSTATE   210
#define BILL_CMD_GETMONEY   220
#define BILL_CMD_STOPMONEY  230
#define BILL_CMD_INHIBIT_FF 300

#define BILL_ERROR_START    25

#define BILL_ST_UNDEFINED  -1
#define BILL_ST_POWERDOWN   0

//
#define BILL_ST_MINOK       1
#define BILL_ST_MAXOK       25

#define BILL_ST_POWERUP     1
#define BILL_ST_POWERUP_BA  2 //with bill in acceptor
#define BILL_ST_POWERUP_BS  3 //with bill in stacker
#define BILL_ST_INIT        4
#define BILL_ST_DISABLE     5
#define BILL_ST_ENABLE      6
#define BILL_ST_ACK         7
#define BILL_ST_ACCEPT      8
#define BILL_ST_STACKING    9
#define BILL_ST_VVALID      10
#define BILL_ST_ECSROW      11
#define BILL_ST_ECSROW_1    12
#define BILL_ST_ECSROW_2    13
#define BILL_ST_ECSROW_3    14
#define BILL_ST_ECSROW_4    15
#define BILL_ST_ECSROW_5    16
#define BILL_ST_ECSROW_6    17
#define BILL_ST_ECSROW_7    18
#define BILL_ST_ECSROW_8    19
#define BILL_ST_ECSROW_9    20
#define BILL_ST_STACKED     21
#define BILL_ST_INHIBIT     22
#define BILL_ST_PAUSE       23
#define BILL_ST_REJECTING   24
#define BILL_ST_RETURN      25
#define BILL_ST_FULL        26
#define BILL_ST_OPEN        27
#define BILL_ST_JAMA        28
#define BILL_ST_JAMS        29
#define BILL_ST_CHEATED     30
#define BILL_ST_FAILURE     40
#define BILL_ST_COERROR     41
#define BILL_ST_INVALIDCMD  42
//
#define BILL_ST_RES1        33
#define BILL_ST_RES2        34
#define BILL_ST_RES3        35
#define BILL_ST_RES4        36
#define BILL_ST_RES5        37
#define BILL_ST_RES6        38
#define BILL_ST_RES7        39

// REJECT
#define BILL_ST_RJCT_MINER          40
#define BILL_ST_RJCT_MAXER          52

#define BILL_ST_RJCT_INSERT         40
#define BILL_ST_RJCT_MAG            41
#define BILL_ST_RJCT_REMAIN1         42
#define BILL_ST_RJCT_COMPENSATION   43
#define BILL_ST_RJCT_CONV           44
#define BILL_ST_RJCT_DENOM          45
#define BILL_ST_RJCT_PHOTOPAT       46
#define BILL_ST_RJCT_PHOTOLEV       47
#define BILL_ST_RJCT_TRANS          48
#define BILL_ST_RJCT_NO             49
#define BILL_ST_RJCT_OPER           50
#define BILL_ST_RJCT_REMAIN         51
#define BILL_ST_RJCT_LEN            52

// FAIL
#define BILL_ST_MINFAIL          53
#define BILL_ST_MAXFAIL          61

#define BILL_ST_FAIL_STACKMOTOR     53 // Stack motor fail A2h
#define BILL_ST_FAIL_FEEDSPD        54 // Transport (feed) motor speed failure A5h
#define BILL_ST_FAIL_FEED           55 // Transport (feed) motor failure A6h
#define BILL_ST_FAIL_NOTREADY       56 // Cash box not ready ABh
#define BILL_ST_FAIL_HEAD           57 // Validator head remove AFh
#define BILL_ST_FAIL_BOOTROM        58 // BOOT ROM failure B0h
#define BILL_ST_FAIL_EXTROM         59 // External ROM failure B1h
#define BILL_ST_FAIL_ROM            60 // ROM failure B2h
#define BILL_ST_FAIL_EXTROMW        61 // External ROM writing failure B3h


/**
 * @brief Bill_OpenDev Abre el puerto y configura la comunicacion ID003.
 * @param dev Direccion del puerto. ej: "/dev/ttyS1"
 * @param Handle apuntador en el que retorna el valor del puerto.
 * @return '1' Si el puerto se abrio de manera exitosa. '0' si ocurrio algun error.
 */
int Bill_OpenDev(char *dev,int* Handle);
/**
 * @brief Bill_CloseDev Cierra el puerto.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return el resultado de la funcion close();
 */
int Bill_CloseDev(int Handle);
/**
 * @brief ReadRespond Recibe la respuesta del Dispositivo. Con time out en microsegundos.
 * @param serial_fd Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param data Apuntador al vector donde se almacenaran los datos leidos.
 * @param size Numero de Bytes a leer.
 * @param timeout_usec Tiempo limite en microsegundos despues de recibir un Byte para erminar la comunicacion si no se recibe otro Byte.
 * @return El numero de Bytes leidos.
 */
int ReadRespond (int serial_fd, char *data, int size, int timeout_usec);
/**
 * @brief ReadRespond2 Recibe la respuesta del Dispositivo. Con time out en segundos.
 * @param serial_fd Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param data Apuntador al vector donde se almacenaran los datos leidos.
 * @param size Numero de Bytes a leer.
 * @param timeout_sec Tiempo limite en segundos despues de recibir un Byte para erminar la comunicacion si no se recibe otro Byte.
 * @return El numero de Bytes leidos.
 */
int ReadRespond2 (int serial_fd, char *data, int size, int timeout_sec);

#ifdef	__cplusplus
}
#endif

#endif	/* IDCOMM_H */

