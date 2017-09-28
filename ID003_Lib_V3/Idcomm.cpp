#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include "IDcomand.h"
#include <pthread.h>
#include "Idcomm.h"

pthread_t thread;


int Bill_OpenDev(char *dev,int* Handle)
{

    printf("\nOpen dev: %s",dev);
    if ((*Handle = open(dev, O_RDWR | O_NOCTTY | O_NDELAY))==-1)
        return 0;
    fcntl(*Handle, F_SETFL, 0);

    struct termios options;

    bzero(&options,sizeof(options));

    options.c_cflag = B9600|CLOCAL|CREAD|PARENB|HUPCL|CS8;
    options.c_iflag  = 0;
    options.c_oflag  = 0;
    options.c_lflag  = 0;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    tcflush(*Handle,TCIFLUSH);
    tcsetattr(*Handle, TCSANOW, &options);
    usleep(250);

    return 1;

}

//-----------------------------------------------------------------------------

int Bill_CloseDev(int Handle)
{
    return close(Handle);

}

//void Bill_GetMoney(int Handlde)
//{
//    terminated = 0;
//    id003_inh(Handlde);
//    SSleep(100);
//    int ret=pthread_create( &thread, NULL, Execute,NULL);
//    return;
//}
////-----------------------------------------------------------------------------
//void Bill_StopMoney(int Handlde)
//{
//    terminated = 1;
//    SSleep(100);
//    id003_inh(Handlde);
//    pthread_join( thread, NULL);
//    return;
//}
//---------
int ReadRespond (int serial_fd, char *data, int size, int timeout_usec)
{
  fd_set fds;
  struct timeval timeout;
  int count=0;
  int ret;
  int n;

  //-- Wait for the data. A block of size bytes is expected to arrive
  //-- within the timeout_usec time. This block can be received as
  //-- smaller blocks.
  do {
      //-- Set the fds variable to wait for the serial descriptor
      FD_ZERO(&fds);
      FD_SET (serial_fd, &fds);

      //-- Set the timeout in usec.
      timeout.tv_sec = 0;
      timeout.tv_usec = timeout_usec;

      //-- Wait for the data
      ret=select (FD_SETSIZE,&fds, NULL, NULL,&timeout);

    //-- If there are data waiting: read it
      if (ret==1) {

        //-- Read the data (n bytes)
        n=read (serial_fd, &data[count], size-count);

        //-- The number of bytes receives is increased in n
        count+=n;

        //-- The last byte is always a 0 (for printing the string data)
        data[count]=0;
      }

    //-- Repeat the loop until a data block of size bytes is received or
    //-- a timeout occurs
  } while (count<size && ret==1);

  //-- Return the number of bytes reads. 0 If a timeout has occurred.
  return count;
}

int ReadRespond2 (int serial_fd, char *data, int size, int timeout_sec)
{
  fd_set fds;
  struct timeval timeout;
  int count=0;
  int ret;
  int n;

  //-- Wait for the data. A block of size bytes is expected to arrive
  //-- within the timeout_usec time. This block can be received as
  //-- smaller blocks.
  do {
      //-- Set the fds variable to wait for the serial descriptor
      FD_ZERO(&fds);
      FD_SET (serial_fd, &fds);

      //-- Set the timeout in sec.
      timeout.tv_sec = timeout_sec;
      timeout.tv_usec = 0;

      //-- Wait for the data
      ret=select (FD_SETSIZE,&fds, NULL, NULL,&timeout);

    //-- If there are data waiting: read it
      if (ret==1) {

        //-- Read the data (n bytes)
        n=read (serial_fd, &data[count], size-count);

        //-- The number of bytes receives is increased in n
        count+=n;

        //-- The last byte is always a 0 (for printing the string data)
        data[count]=0;
      }

    //-- Repeat the loop until a data block of size bytes is received or
    //-- a timeout occurs
  } while (count<size && ret==1);

  //-- Return the number of bytes reads. 0 If a timeout has occurred.
  return count;
}
