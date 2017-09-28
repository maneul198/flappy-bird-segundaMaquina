/******************************************************************************
 *
 * $Id: io_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2008-2014 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI diagnostic demo.
 *
 * License:	GPL v2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Support: Advantech Innocore customers should send e-mail to this address:
 *
 *      support@advantech-innocore.com
 *
 * Users' own modifications to this driver are not supported.
 *
 *****************************************************************************/

#ifdef __linux
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "dpci_version.h"
#include "dpci_core_api.h"

#ifdef __linux
#include <linux/input.h>
#define	INPUT_INTERRUPT_DEVICE	"/dev/input/dpci"
#endif

static int iports, iports_irq, oports;

const unsigned char preset_data_count[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x01, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x09, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x01, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x09, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0x02, 0xf3, 0x04, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0x0a, 0xfb, 0x0c, 0xfd, 0xfe, 0xff,
};

const unsigned char preset_data_bits[] = {
	0x00,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00,
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
	0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f,
	0xff, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0xff,
	0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00,
};

#ifdef WIN32
#define	usleep(us)	Sleep((us) / 1000)
#endif

#define DPCI_DEMO_MENU			0
#define DPCI_DEMO_READ_IO_BOARDIDREV	1
#define DPCI_DEMO_READ_DIS		2
#define DPCI_DEMO_READ_INPORT		3
#define DPCI_DEMO_WRITE_OUTPORT		4
#define DPCI_DEMO_READ_OUTPORT		5
#define DPCI_DEMO_CHANGE_OUTPORT	6
#define DPCI_DEMO_READ_CONT		7
#define DPCI_DEMO_WRITE_CONT		8
#define DPCI_DEMO_VERSION		9
#define DPCI_DEMO_INTERRUPTS		10
#define DPCI_DEMO_PFD			11
#define DPCI_READ_GPIO_INPUT		12
#define DPCI_READ_GPIO_OUTPUT		13
#define DPCI_WRITE_GPIO_OUTPUT		14

static void menu(void)
{
			static const char *api_version_string = NULL;
			static const char *drv_version_string = NULL;
			static unsigned int hwid = 0, rev = 0;

			if (!api_version_string)
			{
				api_version_string = dpci_api_version_string();
				drv_version_string = dpci_drv_version_string();
				dpci_drv_hw_version(&hwid, &rev);
			}
   printf("\n-------------------------------------------------------------------------------\n");
   printf("DirectPCI Demo v" DPCI_VERSION ", $Revision: 12370 $\n");
   printf("(C) 2003-2014 Advantech Co Ltd.  All rights reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
   printf("-------------------------------------------------------------------------------\n");
   printf(" 1. Read I/O board ID and revision\n");
   printf(" 2. Read DIS Switches\n");
   printf(" 3. Read from input port         4. Write to output port\n");
   printf(" 5. Read last output port data   6. Change Bits on Output port\n");
   printf(" 7. Continuous read from input   8. Write data stream to output port\n");
   printf(" 9. Show DirectPCI API and driver versions\n");
   printf("10. Listen for Input Interrupts ");
   printf("11. Listen for PFD Interrupt\n");
   printf("12. Read GPIO inputs            13. Read GPIO outputs\n");
   printf("14. Set GPIO outputs\n");
   printf("Q. Quit (or use Ctrl+C)\n");
   printf("-------------------------------------------------------------------------------\n");
   printf("HW: Type %04x rev.%02x.  %d inputs (%d with interrupts), %d outputs\n",
           hwid, rev, iports * 8, iports_irq * 8, oports * 8);
   printf("-------------------------------------------------------------------------------\n");
   printf("\n");
}

int main(void)
{
   char line[256] = "";
   int cmd = 0;
   int val = 0;
   int delay = 0;
   int newlines = 0, index = 0, count = sizeof(preset_data_bits);
   const unsigned char *data = preset_data_bits;
   unsigned int mask = 0;
   unsigned char val8;
   int port;
   unsigned int set, clear, toggle;

   	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

   iports = dpci_io_numiports(0);
   iports_irq = dpci_io_numiports(1);
   oports = dpci_io_numoports();

   menu();
   while (1)
   {
      printf("Enter option (0 for menu): ");
      cmd = 0;
      fgets(line,sizeof(line),stdin);
      if (tolower(line[0]) == 'q')
      {
         break;
      }
      sscanf(line,"%d", &cmd);
 					
      switch (cmd)
      {
      case DPCI_DEMO_MENU:
         menu();
         break;

      case DPCI_DEMO_READ_IO_BOARDIDREV:
         if (dpci_io_board_supported())
         {
            val = dpci_io_getboard_id();
            if (val == -1)
            {
               perror("Couldn't obtain DPCI I/O board ID");
               break;
            }
            printf("I/O board ID is %d (0x%x)\n", val, val);
            val = dpci_io_getboard_rev();
            if (val == -1)
            {
               perror("Couldn't obtain DPCI I/O board revision");
               break;
            }
            printf("I/O board revision is %d (0x%x)\n", val, val);
         }
         else
         {
            printf("I/O Board not supported on this platform.\n");
         }
         break;

      case DPCI_DEMO_READ_DIS:
         val = dpci_dis_read();
         if (val == -1)
         {
            perror("Can't get switch status");
            break;
         }
	 printf("DIP Switch (SW1) Status:\n");
         printf("SW1.1 is %s\n", val & 1 ? "ON" : "OFF");
         printf("SW1.2 is %s\n", val & 2 ? "ON" : "OFF");
         printf("SW1.3 is %s\n", val & 4 ? "ON" : "OFF");
         printf("SW1.4 is %s\n", val & 8 ? "ON" : "OFF");
 	 break;

      case DPCI_DEMO_READ_INPORT:
	 printf("Enter port to read from (Hex):");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
         if (dpci_io_read_port(port, &val8) == -1)
         {
            perror("Can't read byte from port");
         }
         else
         {
	        printf("Read 0x%02x from port %d\n", val8, port);
         }
	 break;

      case DPCI_DEMO_WRITE_OUTPORT:
	 printf("Enter port to write to (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 printf("Enter value to write to address (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &val);
	 if (dpci_io_write_port(port, (unsigned char)val) == -1)
	 {
	    perror("Can't write byte to port");
	 }
         else
         {
	    printf("Written 0x%02x to port %d\n", val, port);
         }
	 break;

      case DPCI_DEMO_READ_OUTPORT:
	 printf("Enter port to Read from (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 if (dpci_io_read_outport(port, &val8) == -1)
	 {
	    perror("Can't Retrieve last byte written to output port");
	 }
         else
         {
	    printf("Last Byte written to port %d is 0x%02x \n", port, val8);
         }
	 break;

      case DPCI_DEMO_CHANGE_OUTPORT:

	 printf("Enter port to write to (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 printf("Enter bitmask to set (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &set);
	 printf("Enter bitmask to clear (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &clear);
	 printf("Enter bitmask to toggle (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &toggle);

	 if (dpci_io_change_port(port, (unsigned char)set, (unsigned char)clear, (unsigned char)toggle) == -1)
	 {
	    perror("Can't write byte to port");
	 }
         else
         {
	    printf("Set 0x%02x, Cleared 0x%02x, Toggled 0x%02x on port %d\n", set, clear, toggle, port);
         }
	 break;
 	
      case DPCI_DEMO_READ_CONT:
	 printf("Enter port to read from (Hex):");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 printf("Enter delay between accesses (usec):");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%d", &delay);
	 printf("New-lines after every byte? (y/n):");
         fgets(line,sizeof(line),stdin);
         if (tolower(line[0]) == 'y')
            newlines = 1;
         else
            newlines = 0;
         
         while (1)
         {
            if (dpci_io_read_port(port, &val8) == -1)
            {
               perror("Can't read byte from port");
               break;
            }
            printf("Read 0x%02x from port %d", val8, port);
            if (newlines)
               printf("\n");
            else
               printf("\r");
	    if (delay)
            	usleep(delay);
         }
	 break;

      case DPCI_DEMO_WRITE_CONT:
	 printf("Enter port to write to (Hex): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 printf("Enter delay counter between bytes (usec): ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%d", &delay);
	 printf("Enter pattern no (1) bit-strobe, (2) bytes 0-255 (0)=last: ");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%d", &val);
         switch(val)
         {
         case 1:
            data = preset_data_bits;
            count = sizeof(preset_data_bits);
            break;

         case 2:
            data = preset_data_count;
            count = sizeof(preset_data_count);
            break;

         default:
            fprintf(stderr, "No pattern number %d\n", val);
            break;
         }
         while (1)
         {
            val = data[index % count]; 
	    if (dpci_io_write_port(port, (unsigned char)val) == -1)
	    {
	       perror("Can't write byte to port");
               break;
	    }
	    if (delay)
            	usleep(delay);
            printf("Written 0x%02x to port %d\n", val, port);
            index++;
         }
	 break;

      case DPCI_DEMO_VERSION:
         {
            const char *string;
            int code;
            unsigned int hwid, rev;

            code = dpci_api_version();
            string = dpci_api_version_string();
            printf("DPCI API version code:      %08x (%d.%d.%d.%d)\n",
					code,
					DPCI_VERSION_MAJOR(code),
					DPCI_VERSION_MINOR(code),
					DPCI_VERSION_MICRO(code),
					DPCI_VERSION_BUILD(code));
            printf("DPCI API version string:    %s\n", dpci_api_version_string());
            code = dpci_drv_version();
            string = dpci_drv_version_string();
            if (code != -1)
            {
               printf("DPCI driver version code:   %08x (%d.%d.%d.%d)\n", code,
					DPCI_VERSION_MAJOR(code),
					DPCI_VERSION_MINOR(code),
					DPCI_VERSION_MICRO(code),
					DPCI_VERSION_BUILD(code));
            }
            else
            {
               fprintf(stderr,
                       "Couldn't obtain driver driver version code: %s\n",
                       dpci_last_os_error_string());
            }
            if (string != NULL)
            {
               printf("DPCI driver version string: %s\n",dpci_drv_version_string());
            }
            else
            {
               fprintf(stderr,
                       "Couldn't obtain driver driver version code: %s\n",
                       dpci_last_os_error_string());
            }
            if (dpci_drv_hw_version(&hwid, &rev) == 0)
            {
               printf("DPCI HW id:                 %08x\n", hwid);
               printf("DPCI HW rev:                %x\n", rev);
            }
            else
            {
               fprintf(stderr,
                       "Couldn't obtain driver hardware id and revision: %s\n",
                       dpci_last_os_error_string());
            }
         }
         break;
		
 
      case DPCI_DEMO_INTERRUPTS: 
	 printf("Enter port to monitor (Hex):");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &port);
	 printf("Enter mask of bits to monitor (Hex,ff=all bits):");
         fgets(line,sizeof(line),stdin);
         sscanf(line,"%x", &mask);
         if (mask == 0)
            mask = 0xff;

         {
            unsigned char int_pol;
            dpci_io_portevent_t digip_data;

	    printf("\nListening for Input interrupts (Ctrl+C to stop)\n");
            int_pol = mask;
            while (1)
            {
               int int_mask;
               unsigned char ip;

               memset(&digip_data, 0, sizeof(digip_data));
               int_mask = dpci_io_wait_port(port, (unsigned char)mask, int_pol, 30000);
               if (int_mask < 0)
               {
                  perror("dpci_io_wait_port()");
                  break;
               }
               if (int_mask == 0)
               {
                  printf("Timed-out\n");
                  continue;
               }
               dpci_io_read_port(port, &ip);
               int_pol ^= int_mask;
               printf("change-mask=0x%02x ip%d=0x%02x int_pol=0x%02x\n", int_mask, port, ip, int_pol);
            }
         }
         break;

	case DPCI_DEMO_PFD:
	    printf("\nListening for PFD interrupt (Ctrl+C to stop)\n");
            while (1)
            {
               int int_mask;
   
               int_mask = dpci_io_wait_pfd(30000);
               if (int_mask < 0)
               {
                  perror("dpci_io_wait_pfd()");
                  break;
               }
               if (int_mask == 0)
               {
                  printf("Timed-out\n");
                  continue;
               }
               printf("Power Fail Detected\n");
            }
  	    break; 

	case DPCI_READ_GPIO_INPUT:
	    {
		    int rc;
		    unsigned char value;
		    int i;

		    for(i=0; i<2; i++)
		    {
			    rc = dpci_gpio_read_ip(i, &value);
			    if(rc != 0)
			    {
				    perror("dpci_gpio_read_ip");
			    }
			    else
			    {
				    printf("GPIO line %d is %s\n", i, value ?  "set" : "unset");
			    }
		    }
	    }
	    break;

	case DPCI_READ_GPIO_OUTPUT:
	    {	
		    int rc;
		    unsigned char value;
		    int i;

		    for(i=0; i<2; i++)
		    {
			    rc = dpci_gpio_read_op(i, &value);
			    if(rc != 0)
			    {
				    perror("dpci_gpio_read_op");
			    }
			    else
			    {
				    printf("GPIO line %d (output) is %s\n", i, value ?  "set" : "unset");
			    }
		    }
	    }
	    break;

	case DPCI_WRITE_GPIO_OUTPUT:
	    {
		    int rc = 0;
		    int i;
		    int set;

		    for(i=0; i<2; i++)
		    {

			    printf("Enter value for GPIO line %d [0..1]:", i);
			    fflush(stdout);
			    if(fgets(line,sizeof(line),stdin) == NULL)
			    {
				    break;
			    }
			    rc = sscanf(line,"%x", &set);
			    if(rc != 1)
			    {
				    break;
			    }

			    rc = dpci_gpio_write_op(i, set != 0);
			    if(rc != 0)
			    {
				    perror("dpci_gpio_write_op");
			    }
		    }
	    }
	    break;

	  default:
	 fprintf(stderr, "Not implemented.\n");
      } 
   }
   return 0;
}
