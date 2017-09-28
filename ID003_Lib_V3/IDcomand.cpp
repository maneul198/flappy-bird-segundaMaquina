#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "IDcomand.h"


int State   = 0;
int StateEx = 0;

int tdinero;
/*-------------------------CRC para ID003--------------------------*/

const int TABLE[256] = {
  0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
  0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
  0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
  0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
  0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
  0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
  0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
  0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
  0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
  0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
  0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
  0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
  0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
  0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
  0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
  0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
  0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
  0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
  0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
  0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
  0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
  0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
  0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
  0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
  0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
  0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
  0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
  0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
  0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
  0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
  0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
  0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};
void CRCID003(unsigned char*buf,int longitud,unsigned char* salida)
{
    extern const int TABLE[256];
    int crct=0,table;
    int i;
    for (i = 0; i < longitud; ++i){
        table=TABLE[buf[i]^(crct&0x00ff)];
        crct = (crct>>8)^table;
    }
    salida[0]=(char)(crct&0x00ff);
    salida[1]=(char)((crct&0xff00)>>8);
};


/*--------------------Envio de Comandos---------------------------*/
int bill_Do_Cmd(int hndl,unsigned char* cmd,int len)
    {
    if (hndl<1) return 0;
    if (len<1) return 0;
    if (cmd == NULL) return 0;

    int bw = write(hndl, cmd, len);

    if ((bw < 0)||(bw!=len))
    {
        perror("Write to port fail");
        printf("\nОшибка записи в порт!\n");
        return 0;
    }

    return 1;
    }
/*------------------Comandos Operativos---------------------------*/
int id003_poll(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x11,0x27,0x56};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_reset(int Handle){
    unsigned char buf[5] = {0xFC,0x05,0x40,0x2B,0x15};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_stack_1(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x41,0xa2,0x04};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_stack_2(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x42,0x39,0x36};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_stack_3(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x49,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,3,salida);
    buf[3]=salida[0];
    buf[4]=salida[1];
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_Ack(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x50,0xaa,0x05};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_retorno(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x43,0xb0,0x27};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_hold(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x44,0x0f,0x53};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_waitb(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x45,0x86,0x42};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_ReqbarF(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x86,0x11,0xb6};
    return bill_Do_Cmd(Handle,buf,5);
}
int id003_Reqbari(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x87,0x98,0xa7};
    return bill_Do_Cmd(Handle,buf,5);
}

/*-------------------------------*Comandos extencion reciclador----------------*/
int id003_Pay_Out(int Handle,int recilador,int billnum)
{
    unsigned char buf[9] = {0xfc,0x09,0xf0,0x20,0x4a,billnum,recilador,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,7,salida);
    buf[7]=salida[0];
    buf[8]=salida[1];
    return bill_Do_Cmd(Handle,buf,9);
}

int id003_Collect(int Handle,int recilador,int billnum)
{
    //billnum=1 ==>colect one banknote
    //billnum=0 ==>colect all banknote
    unsigned char buf[9] = {0xfc,0x09,0xf0,0x20,0x4b,billnum,recilador,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,7,salida);
    buf[7]=salida[0];
    buf[8]=salida[1];
    return bill_Do_Cmd(Handle,buf,9);
}

int id003_Clear(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x4c,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Emergency_Stop(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x4d,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}
/*-------------------------------*Comandos de configuracion--------------------*/

int id003_enable(int Handle)
{
    unsigned char buf[7] = {0xFC,0x07,0xc0,0xc0,0x00,0xff,0x70};
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_enable2(int Handle,char habilitados)
{
    unsigned char buf[7] = {0xFC,0x07,0xc0,habilitados,0x00,0xff,0x70};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);

}

int id003_Barkodeinh(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc7,0xfc,0x87,0x8c};
    return bill_Do_Cmd(Handle,buf,6);
}

int id003_Barkfun(int Handle)
{
    unsigned char buf[7] = {0xFC,0x07,0xc6,0x01,0x12,0xbf,0x49};
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_commode(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc2,0x00,0xdc,0xfc};
    return bill_Do_Cmd(Handle,buf,6);
}


int id003_conmode2(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc2,0x01,0x55,0xde};
    return bill_Do_Cmd(Handle,buf,6);
}

int id003_conmode3(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc2,0x02,0xce,0xec};
    return bill_Do_Cmd(Handle,buf,6);
}

int id003_seguridad(int Handle)
{
    unsigned char buf[7] = {0xFC,0x07,0xc1,0x00,0x00,0xf1,0xef};
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_funcionop(int Handle)
{
    unsigned char buf[7] = {0xFC,0x07,0xc6,0x00,0x00,0xf4,0x63};
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_inh(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc3,0x00,0x04,0xd6};
    return bill_Do_Cmd(Handle,buf,6);
}

int id003_inh_FF(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc3,0x01,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,4,salida);
    buf[4]=salida[0];
    buf[5]=salida[1];
    return bill_Do_Cmd(Handle,buf,6);
}

int id003_Direction(int Handle)
{
    unsigned char buf[6] = {0xFC,0x06,0xc4,0x00,0x0c,0x96};
    return bill_Do_Cmd(Handle,buf,6);
}
/*----------------------------------------Extencion de comandos de comunicacion----------------------------------*/

int id003_Recicler_Currency(int Handle,int den1,int den2)
{
    unsigned char buf[13] = {0xfc,0x0D,0xf0,0x20,0xd0,den1,0x00,0x00,den2,0x00,0x01,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,11,salida);
    buf[11]=salida[0];
    buf[12]=salida[1];
    return bill_Do_Cmd(Handle,buf,13);
}

int id003_Recicler_Key(int Handle,int key)
{
    unsigned char buf[8] = {0xfc,0x08,0xf0,0x20,0xd1,key,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,6,salida);
    buf[6]=salida[0];
    buf[7]=salida[1];
    return bill_Do_Cmd(Handle,buf,8);
}

int id003_Recicler_count(int Handle,char* data)
{
    unsigned char buf[10] = {0xfc,0x0a,0xf0,0x20,0xd2,data[0],data[1],data[2],0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,8,salida);
    buf[8]=salida[0];
    buf[9]=salida[1];
    return bill_Do_Cmd(Handle,buf,10);
}

int id003_Recicler_current_count(int Handle,char* data)
{
    unsigned char buf[10] = {0xfc,0x0a,0xf0,0x20,0xe2,data[0],data[1],data[2],0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,8,salida);
    buf[8]=salida[0];
    buf[9]=salida[1];
    return bill_Do_Cmd(Handle,buf,10);
}
/*------------------------Unit information request-------------------------------------------*/

int id003_unit_information(int Handle){
    unsigned char buf[5] = {0xfc,0x05,0x92,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,3,salida);
    buf[3]=salida[0];
    buf[4]=salida[1];
    return bill_Do_Cmd(Handle,buf,5);
}

/*--------------------------------estatus Recuest extention-----------------------------------*/

int id003_status_recuest_Ex(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x1a,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}


int id003_RCurency_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x90,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_count_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x92,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_softwareVercion_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x93,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_Total_count_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0xa0,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_Total_count_clear(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0xa1,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_Current_count_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0xa2,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}

int id003_Recicler_key_setting_recuest(int Handle)
{
    unsigned char buf[7] = {0xfc,0x07,0xf0,0x20,0x91,0x00,0x00};
    unsigned char salida[2];
    CRCID003(buf,5,salida);
    buf[5]=salida[0];
    buf[6]=salida[1];
    return bill_Do_Cmd(Handle,buf,7);
}
/*---------------funcion dinero-------------------*/

int dinfun(char sdin){
    int tdinero=0;
    if (sdin==97)
        tdinero=1000;
    else if (sdin==98)
        tdinero=2000;
    else if (sdin==99)
        tdinero=5000;
    else if (sdin==100)
        tdinero=10000;
    else if (sdin==101)
        tdinero=20000;
    else if (sdin==102)
        tdinero=50000;
    return tdinero;
}

