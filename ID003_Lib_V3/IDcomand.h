/*
 * File:   IDcomand.h
 * Author: cabitech
 *
 * Created on 25 de noviembre de 2015, 04:16 PM
 */

#ifndef IDCOMAND_H
#define	IDCOMAND_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief CRCID003 Esta funcion calcula el CRC del protocolo ID003, que se coloca al final de cada intruccion como control de errores.
 * @param buf Este parametro, es un apuntador a la primera direccion de memoria del arreglo donde se almacenan los Bytes que conforman la instruccion a enviar.
 * @param longitud Este parametro en un entero e indica la longitud de el vector que contiene la instruccion.
 * @param salida Es un apuntador a la primera direcion en memoria donde se almacena el vector de salida. Recuerde que la longitud de salida es de 2 Bytes.
 */
void CRCID003(unsigned char*buf,int longitud,unsigned char* salida);
/**
 * @brief bill_Do_Cmd Esta funcion envia la intruccion por serial al dispositivo.
 * @param hndl Este parametro es un entro y es el manejador del puerto por el cual se va a enviar el comando. Recuerde que el manejador lo openemos al abrir el puerto.
 * @param cmd Este comando es un apuntador a la priemra direccion de memoria del vector que se desea enviar al dispositivo.
 * @param len Es la longitud del vector que se esta enviando.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 */
int bill_Do_Cmd(int hndl,unsigned char* cmd,int len);
/*-------------------------------Comandos operativos--------------------------*/
/**
 * @brief id003_poll Este comando hace un pol al dispositivo.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * Retorna el estado en que se encuentra el dispositivo. Recuerde consultar el manual ID003 para poder identificar el estado.
 */
int id003_poll(int Handle);
/**
 * @brief id003_reset Envia la orden de reset al dispositivo.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de reset.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_reset(int Handle);
/**
 * @brief id003_stack_1 Envia la orden de guardar el billete o TITO en el cashbox, Este comando solo es valido cuando el dispositivo esta en estado de 'Escrow'
 * Cuando se recive este comando, el vilidador solo pasara a estado 'vend valid' caundo el billete o ticket este amedio camino al cashbox.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de stack_1.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_stack_1(int Handle);
/**
 * @brief id003_stack_2 Envia la orden de guardar el billete o TITO en el cashbox, Este comando solo es valido cuando el dispositivo esta en estado de 'Escrow'
 * Cuando se recive este comando, el vilidador solo pasara a estdo 'vend valid' caundo el billete o ticket este en el Cashbox.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de stack_2.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_stack_2(int Handle);
/**
 * @brief id003_stack_3 Envia la orden de guardar el billete o TITO en el cashbox sin importar si es una denominacion selecionada para reciclaje.
 * Este comando solo es valido cuando el dispositivo esta en estado de 'Escrow' Cuando se recive este comando,
 * el vilidador solo pasara a estdo 'vend valid' caundo el billete o ticket este en el Cashbox.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de stack_2.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_stack_3(int Handle);
/**
 * @brief id003_Ack Comando de reconocimiento, reconoce el estado 'vendvalid'.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 */
int id003_Ack(int Handle);
/**
 * @brief id003_retorno Comando que ordena al aceptador el devolver la nota o el tiket. Este comando solo es valido cuando el dispositivo esta en estado de 'Escrow'.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de RETURN.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_retorno(int Handle);
/**
 * @brief id003_hold Comando que ordena al dispositivo retener la nota o el tiquet por 10 segundos. Este comando solo es valido cuando el dispositivo esta en estado de 'Escrow'.
 * el dispositivo expulsara el billete o ticket si el comando no es repetido antes de 10 segundos, recuerde que para almacenarlo debe dar la orden de Stack.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de HOLD.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_hold(int Handle);
/**
 * @brief id003_waitb Comando para detener el acepador por 3 segundos, recuerde que este comando puede enviarse multiples veces.
 * Este comando es valido cuando un billete o ticket esta siendo procesado. Para mas informacion consulte el manual ID003 de JCM global.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * La respuesta del dispositivo al comando de HOLD.
 * Retorna '80' (0x50) si es un ACK o '75' (0x4B) en caso de ser un comando invalido.
 */
int id003_waitb(int Handle);
/**
 * @brief id003_ReqbarF Este comando envia una peticion al aceptador, indagando por el numero de caracteres y tipo de codigo de Barras configurados en el aceptador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '134' (0x86) + Datos. Revisar el manual ID003 apendice D, III-B-1.
 */
int id003_ReqbarF(int Handle);
/**
 * @brief id003_Reqbari Este comando envia una peticion al aceptador, indagando por la configuracion del aceptador (si esta habilitado el codigo de varras o la entrada de billetes)
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '135' (0x87) + Datos. Revisar el manual ID003 apendice D, III-B-2.
 */
int id003_Reqbari(int Handle);

/*-------------------------------*Comandos extencion reciclador----------------*/
/**
 * @brief id003_Pay_Out Comando que pide al reciclador entregar un numero de billetes. Recuerde que este comando solo puede ser enviado cuando el acetador este en el estado 'desavilitado'.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param recilador Indica el recilado del que se entregaran billetes. Recicladores 0x00 y 0x01.
 * @param billnum Indica el nuemro de billetes a entregar. Algunos dispositivo pueden no soportar la entrega de multiples billetes.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '80' (0x50) En caso de aceptar el comando.'75' (0x4B) Comando invalido en caso de no estar deshabilitado el reciclador.
 */
int id003_Pay_Out(int Handle,int recilador,int billnum);
/**
 * @brief id003_Collect El reciclador enviara los billetes de las caseteras de resiclaje  determinada al Cashbox. Recuerde que este comando solo puede ser enviado cuando el acetador este en el estado 'desavilitado'.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param recilador Indica el reciclador del que se quiere hacer la recoleccion. Recicladores 0x00 y 0x01.
 * @param billnum Indica la cantiad de billetes a enviar al Cashbox.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '80' (0x50) En caso de aceptar el comando.'75' (0x4B) Comando invalido en caso de no estar deshabilitado el reciclador.
 */
int id003_Collect(int Handle,int recilador,int billnum);
/**
 * @brief id003_Clear Limpia los errores de reciclado. El host debe enviar este comando encaso de presentarce algun problema en el reciclado.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '80' (0x50) En caso de aceptar el comando.
 */
int id003_Clear(int Handle);
/**
 * @brief id003_Emergency_Stop Este comando se usa para detener un pago, enviando el billete al cashbox. este comando solo puede ser usado antes de entrar en estado 'Pay Valid'.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '80' (0x50) En caso de aceptar el comando.
 */
int id003_Emergency_Stop(int Handle);
/*-------------------------Comandos de configuracion--------------------------*/
/**
 * @brief id003_enable Este comando habilita o deshabilita cada una de las denominaciones soportadas por el validador. envia 1100 0000. '0' enable '1' disable.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '192' (0xCO) + Data.
 */
int id003_enable(int Handle);
/**
 * @brief id003_enable2 Este comando habilita o deshabilita cada una de las denominaciones soportadas por el validador. '0' enable '1' disable.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param habilitados Envia el Byte con las denominaciones a habilitar o deshabilitar.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '192' (0xCO) + Data.
 */
int id003_enable2(int Handle,char habilitados);
/**
 * @brief id003_Barkodeinh Este comando habilita la lectura de ticket y de Billetes.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '199' (0xC7) + Data.
 */
int id003_Barkodeinh(int Handle);
/**
 * @brief id003_Barkfun Esta funcion configura el codigo de barras para "interleaved 2 of 5" y 18 caracteres.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '198' (0xC6) + Data.
 */
int id003_Barkfun(int Handle);
/**
 * @brief id003_commode Configura el modo de comunicacion por Polling.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '194' (0xC2) + Data.
 */
int id003_commode(int Handle);
/**
 * @brief id003_conmode2 Configura el modo de comunicacion modo de interrupcion 1.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '194' (0xC2) + Data.
 */
int id003_conmode2(int Handle);
/**
 * @brief id003_conmode3 Configura el modo de comunicacion modo de interrupcion 2.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '194' (0xC2) + Data.
 */
int id003_conmode3(int Handle);
/**
 * @brief id003_seguridad Configura el nivel de seguridad para la validacion de dispositivos. En este caso seguridad estandard
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '193' (0xC1) + Data. Si el no se envia durante la inicializacion respondera comando invalido '75' (0x4B)
 */
int id003_seguridad(int Handle);
/**
 * @brief id003_funcionop deshabilita las funciones opcionales. Esta funcion esta en default.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '197' (0xC5) + Data. Si el no se envia durante la inicializacion respondera comando invalido '75' (0x4B)
 */
int id003_funcionop(int Handle);
/**
 * @brief id003_inh habilita la aceptacion de todo billete o cupon. Habilita el aceptador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '195' (0xC3) + Data. Si el no se envia durante la inicializacion respondera comando invalido '75' (0x4B)
 */
int id003_inh(int Handle);
/**
 * @brief id003_inh_FF deshabilita la aceptacion de todo billete o cupon. Deshabilita el aceptador.
 * @param Handle  Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '195' (0xC3) + Data. Si el no se envia durante la inicializacion respondera comando invalido '75' (0x4B)
 */
int id003_inh_FF(int Handle);
/**
 * @brief id003_Direction configura sentido de aceptacion, En especifico los billetes son aceptados sin importar enque direccion y sentido se ingresen.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '196' (0xC4) + Data. Si el no se envia durante la inicializacion respondera comando invalido '75' (0x4B)
 */
int id003_Direction(int Handle);
/*----------------------------------------Extencion de comandos de comunicacion----------------------------------*/
/**
 * @brief id003_Recicler_Currency Configura las denominaciones a recilcar.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param den1 Denominacion numero 1. Recuerde mirar el manual ID003 para usar este coamndo.
 * @param den2 Denominacion numero 2. Recuerde mirar el manual ID003 para usar este coamndo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 208 (0xD0) + Data.
 */
int id003_Recicler_Currency(int Handle,int den1,int den2);
/**
 * @brief id003_Recicler_Key Dirigirce al manual del ID003 del reciclador de la empresa JCM global.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param key Consultar manual.
 * @return '240' (0xf0) + 209 (0xD1) + Data.
 */
int id003_Recicler_Key(int Handle,int key);
/**
 * @brief id003_Recicler_count Configura el maximo numero de billetes por reciclador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param data Apuntador a un vector de 3 posiciones de tipo Byte, En el primera posicion debe ir el numero de maximo de billetes.
 * En la segunda pocicion debe ir 0x00 y en la tercera posicion el reciclador.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 300 (0xD2) + Data.
 */
int id003_Recicler_count(int Handle,char* data);
/**
 * @brief id003_Recicler_current_count Configura el numero de billetes almacenadas actualmente en l casetera de reciclaje.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @param data Apuntador a un vector de 3 posiciones de tipo Byte, En el primera posicion debe ir el numero de billetes actualmente en el reciclador.
 * En la segunda pocicion debe ir 0x00 y en la tercera posicion el reciclador.
 * @return '240' (0xf0) + 226 (0xE2) + Data.
 */
int id003_Recicler_current_count(int Handle,char* data);

/*------------------------Unit information request-------------------------------------------*/
/**
 * @brief id003_unit_information Comando para obtener la informacion del reciclador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * 146 (0x92)+Data. Consulte ID003 JCM global Manual.
 */
int id003_unit_information(int Handle);
/*--------------------------------estatus Recuest extention-----------------------------------*/
/**
 * @brief id003_status_recuest_Ex Indaga por el estado del reciclador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * Consultar Manual de comunicaciones ID003 extencion para recicladores.
 */
int id003_status_recuest_Ex(int Handle);
/**
 * @brief id003_RCurency_recuest Indaga por las Denominaciones configuradas para reciclar.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 226 (0xE2) + Data.
 */
int id003_RCurency_recuest(int Handle);
/**
 * @brief id003_Recicler_count_recuest Indaga por la cantidad de billetes maxima configurada en el resiclador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 146 (0x92) + Data.
 */
int id003_Recicler_count_recuest(int Handle);
/**
 * @brief id003_Recicler_softwareVercion_recuest Obtener la version de sofware del reciclador.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return  Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 147 (0x93) + Data.
 */
int id003_Recicler_softwareVercion_recuest(int Handle);
/**
 * @brief id003_Recicler_Total_count_recuest Obtener el total de billetes Reciclados, Enviados al Cashbox y Recolectados.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 160 (0xA0) + Data.
 */
int id003_Recicler_Total_count_recuest(int Handle);
/**
 * @brief id003_Recicler_Total_count_clear Borra la cuenta total de billetes Reciclados, Enviados al Cashbox y Recolectados.
 * @param Handle Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 161 (0xA1) + Data.
 */
int id003_Recicler_Total_count_clear(int Handle);
/**
 * @brief id003_Recicler_Current_count_recuest Obtener el numero de billetes en los recicladores.
 * @param Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 162 (0xA2) + Data.
 */
int id003_Recicler_Current_count_recuest(int Handle);
/**
 * @brief id003_Recicler_key_setting_recuest Indaga por el estado del Inhibit o Recicler key.
 * @param Este paramentro es el manejador del puerto donde esta conectado el dispositivo.
 * @return Retorna '0' en caso de ocurrir algun error al escribir los datos en el puerto. Retorna '1' si el vector a enviar se escribio completamente en el puerto.
 * '240' (0xf0) + 163 (0xA3) + Data.
 */
int id003_Recicler_key_setting_recuest(int Handle);
/*---------------funcion dinero-------------------*/
/**
 * @brief dinfun Toma el Codigo de escrow entregado por el validador y devuelve la cantidad de dinero al que corresponde.
 * @param sdin Codigo entregado por el validador.
 * @return Valor en pesos.
 */
int dinfun(char sdin);






#ifdef	__cplusplus
}
#endif

#endif	/* IDCOMAND_H */

