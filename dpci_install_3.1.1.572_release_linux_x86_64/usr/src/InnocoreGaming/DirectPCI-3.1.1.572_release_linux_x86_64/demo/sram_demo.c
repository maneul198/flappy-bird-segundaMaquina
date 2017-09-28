/******************************************************************************
 *
 * $Id: sram_demo.c 11488 2015-01-31 23:25:03Z aidan $
 *
 * Copyright 2003-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * Advantech Innocore DirectPCI SRAM demo.
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

#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <dpci_sram_api.h>

#define SRAM_MENU			0
#define SRAM_SIZE			1
#define SRAM_BITWALKTEST	2
#define SRAM_ADDRESSTEST	3
#define SRAM_READ_BYTE		4
#define SRAM_WRITE_BYTE		5
#define SRAM_READ_WORD		6
#define SRAM_WRITE_WORD		7
#define SRAM_READ_DWORD		8
#define SRAM_WRITE_DWORD	9
#define SRAM_CLEAR			10
#define SRAM_DUMP			11
#define SRAM_WRITE_STRING	12
#define SRAM_DIRECT_DUMP		13


/*******************************************************************************
 *
 * Function:	menu()
 *
 * Parameters:  none.
 *
 * Returns:	 nothing.
 *
 * Description: prints a menu of options.
 *
 ******************************************************************************/
static void menu(void)
{
	printf("\n--------------------------------------------------------------------------------\n");
	printf("DirectPCI SRAM API Demo $Revision: 11488 $\n");
	printf("Copyright 2004-2015 2013Ltd. All Rights Reserved\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("1.  Get SRAM size\n");
	printf("2.  Bit-walk test                  3. Shadow test.\n");
	printf("4.  Read byte from SRAM            5. Write byte to SRAM.\n");
	printf("6.  Read word from SRAM            7. Write word to SRAM.\n");
	printf("8.  Read dword from SRAM           9. Write dword to SRAM.\n");
	printf("10. Clear SRAM.                    11.Dump SRAM contents.\n");
	printf("12. Write string to SRAM.          13.Dump SRAM (mapped)\n");
	printf("Q.  Quit.\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("\n");
}


/*******************************************************************************
 *
 * Function:	sram_read_sized()
 *
 * Parameters:  offset - the offset in SRAM to read from
 *			  size - the size of the data to read, in bytes.
 *
 * Returns:	 unsigned int - the data obtained.
 *
 * Description: obtains data in requested format and returns as unsigned int.
 *
 ******************************************************************************/
static unsigned int sram_read_sized(unsigned int offset, int size)
{
	unsigned char val8 = 0xff;
	unsigned short val16 = 0xffff;
	unsigned int val32 = 0xffffffff;

	switch (size)
	{
	case 1:
		dpci_sram_read8(offset, &val8);
		return val8;
	case 2:
		dpci_sram_read16(offset, &val16);
		return val16;
	case 4:
		dpci_sram_read32(offset, &val32);
		return val32;
	}
	printf("Invalid word-size %d in sram_read_sized.\n", size);
	return 0xffffffff;
}


/*******************************************************************************
 *
 * Function:	sram_write_sized()
 *
 * Parameters:  offset - the offset in SRAM to write to
 *			  data - data to write at offset
 *			  size - the size of the data to read, in bytes.
 *
 * Returns:	 int - status code from call or -1 for error.
 *
 * Description: writes data in requested format and returns status.  Data will
 *			  be truncated to size given, with MSBs being lost.
 *
 ******************************************************************************/
static int sram_write_sized(unsigned int offset, unsigned int data, int size)
{
	switch (size)
	{
	case 1:
		return dpci_sram_write8(offset, (unsigned char)data);
	case 2:
		return dpci_sram_write16(offset, (unsigned short)data);
	case 4:
		return dpci_sram_write32(offset, data);
	}
	printf("Invalid word-size %d in sram_read_sized.\n", size);
	return -1;
}


/*******************************************************************************
 *
 * Function:	addresstest()
 *
 * Parameters:  none.
 *
 * Returns:	 nothing.
 *
 * Description: A test to see if any address lines are shorting.  Clears the
 *			  entire SRAM contents and the writes data to selected addresses.
 *		Subsequent reads of the entire SRAM should correctly read the
 *			  last data written at that location.
 *
 *			  Presently no diagnosis is made of errors found.
 *
 *			  If errors occur during the preparation (clearing or writing
 *			  of test data) stage then the test is aborted.
 *
 ******************************************************************************/
static void addresstest(void)
{
#define	TEST_DATA0	0xa5a5a5a5
#define	TEST_DATA1	0xf0f0f0f0
#define	TEST_DATA2	0x5a5a5a5a
#define	TEST_DATA3	0x0f0f0f0f
	char line[256];
	unsigned int start_addr = 0, end_addr = dpci_sram_size(), addr, test_addr;
	int dsize = 16;
	unsigned int dmask, d0, d1, d2, d3;
	int steps = 1, stepno;
	int errors = 0;

	/*
	 * Sanity check end_addr.
	 */
	if (end_addr == 0)
	{
		printf("Can't automatically find SRAM size.\n");
		printf("Enter end address to test (Hex - default %x):", end_addr);
		fgets(line,sizeof(line),stdin);
		sscanf(line,"%x", &end_addr);
	}

	/*
	 * Get chosen data size and set some useful variables.
	 */
	printf("Enter data size to test (8, 16 or 32 bits - default 16): ");
	fgets(line,sizeof(line),stdin);
	sscanf(line,"%d", &dsize);
	switch (dsize)
	{
	case 8:
		dmask = 0xff;
		break;
	case 16:
		dmask = 0xffff;
		break;
	case 32:
		dmask = 0xffffffff;
		break;
	default:
		printf("Data size %d not supported.\n", dsize);
		return;
	}

	/*
	 * Get number of steps to run.
	 */
	printf("Enter numbers of steps to run (default 1): ");
	fgets(line,sizeof(line),stdin);
	sscanf(line,"%d", &steps);

	/*
	 * Convert to bytes for incrementing and calling sram access functions
	 * above.
	 */
	dsize /= 8;

	for (stepno = 0; stepno < steps; stepno++)
	{
		/*
		 * Clear SRAM.
		 */
		printf("Clearing SRAM (%d-bit writes) from %x to %x: ",
					dsize * 8, start_addr, end_addr);
		fflush(stdout);
		for (addr = start_addr; addr < end_addr; addr += dsize)
		{
			/*
			 * If we've reached a particular interval in the test,
			 * then output a progress report.
			 */
			if ((addr & 0x1ff) == 0)
			{
				printf("%06x\b\b\b\b\b\b", addr);
				fflush(stdout);
			}
	
			if (sram_write_sized(addr, 0, dsize) == -1)
			{
				printf("\nFailed to write to SRAM at %x: %s\n",
					addr, dpci_last_os_error_string());
				errors++;
				break;
			}
			if ((d0 = sram_read_sized(addr, dsize)) != 0)
			{
				printf("\nSRAM clear failed at %x: got %x\n",
					addr, d0);
				errors++;
			}
		}
		printf("done.   \n");
		if (errors)
		{
			printf("*** %d errors just clearing SRAM - aborting\n",
				errors);
			return;
		}
	
		/*
		 * Write some initial test data.
		 */
		printf("Writing test data... ");
		fflush(stdout);
		test_addr = start_addr + (dsize * stepno);
		sram_write_sized(test_addr, TEST_DATA0, dsize);
		sram_write_sized(test_addr + dsize, TEST_DATA1, dsize);
		sram_write_sized(test_addr + dsize * 2, TEST_DATA2, dsize);
		sram_write_sized(test_addr + dsize * 3, TEST_DATA3, dsize);
		printf("done\n");
	
		/*
		 * Check the initial test data.
		 */
		printf("Checking test data... ");
		fflush(stdout);
		d0 = sram_read_sized(test_addr, dsize);
		if (d0 != (TEST_DATA0 & dmask))
		{
			printf("Initial test data0 readback bad: "
				"wanted %x got %x\n",
				TEST_DATA0 & dmask, d0);
			errors++;
		}
		d1 = sram_read_sized(test_addr + dsize, dsize);
		if (d1 != (TEST_DATA1 & dmask))
		{
			printf("Initial test data1 readback bad: "
				"wanted %x got %x\n",
				TEST_DATA1 & dmask, d1);
			errors++;
		}
		d2 = sram_read_sized(test_addr + dsize * 2, dsize);
		if (d2 != (TEST_DATA2 & dmask))
		{
			printf("Initial test data2 readback bad: "
				"wanted %x got %x\n",
				TEST_DATA2 & dmask, d2);
			errors++;
		}
		d3 = sram_read_sized(test_addr + dsize * 3, dsize);
		if (d3 != (TEST_DATA3 & dmask))
		{
			printf("Initial test data3 readback bad: "
				"wanted %x got %x\n",
				TEST_DATA3 & dmask, d3);
			errors++;
		}
		printf("done\n");
		if (errors)
		{
			printf("*** %d errors checking test data - aborting\n",
				errors);
			return;
		}

		/*
		 * Now try and find some non-zero bytes in the rest of the SRAM.
		 * Start just after the test data we just read and checked.
		 */
		printf("Checking remaining SRAM data: ");
		fflush(stdout);
		for (addr = test_addr + 4 * dsize;
						addr < end_addr;
						addr += dsize)
		{
			/*
			 * If we've reached a particular interval in the test,
			 * then output a progress report.
			 */
			if ((addr & 0x1ff) == 0)
			{
				printf("%06x\b\b\b\b\b\b", addr);
				fflush(stdout);
			}
	
			d0 = sram_read_sized(addr, dsize);
			if (d0 != 0)
			{
				printf("\nData readback bad at %08x: "
					"wanted 0 got %x\n",
					addr, d0);
				errors++;
			}
		}
		printf("done.  \n");
		if (errors)
		{
			printf("*** %d errors checking cleared SRAM\n", errors);
		}
	}
}


/*******************************************************************************
 *
 * Function:	bitwalktest()
 *
 * Parameters:  none.
 *
 * Returns:	 nothing.
 *
 * Description: A test to see if any data lines are shorting.  Walks bit-by-bit
 *			  through the entire SRAM contents writing each bit only at a time
 *			  and checking only that bit is still set on read-back.
 *
 ******************************************************************************/
static void bitwalktest(void)
{
	char line[256];
	unsigned int orig_data, bit, readback;
	unsigned int start_addr = 0, end_addr = dpci_sram_size(), addr;
	int dsize = 16;
	int bitmax;

	/*
	 * Get start address - default is start of SRAM.
	 */
	printf("Enter start address to test (Hex - default %x): ", start_addr);
	fgets(line,sizeof(line),stdin);
	sscanf(line,"%x", &start_addr);

	/*
	 * Get end address - default is end of SRAM.
	 */
	printf("Enter end address to test (Hex - default %x):", end_addr);
	fgets(line,sizeof(line),stdin);
	sscanf(line,"%x", &end_addr);

	/*
	 * Get chosen data size and set some useful variables.
	 */
	printf("Enter data size to test (8, 16 or 32 bits - default 16): ");
	fgets(line,sizeof(line),stdin);
	sscanf(line,"%d", &dsize);
	switch (dsize)
	{
	case 8:
		bitmax = 0x80;
		break;
	case 16:
		bitmax = 0x8000;
		break;
	case 32:
		bitmax = 0x80000000;
		break;
	default:
		printf("Data size %d not supported.\n", dsize);
		return;
	}

	/*
	 * Convert to bytes for incrementing and calling sram access functions
	 * above.
	 */
	dsize /= 8;

	/*
	 * Go through chosen address range.
	 */
	printf("Starting bit-walk test of SRAM from %06x to %06x.\n",
		start_addr, end_addr);
	for (addr = start_addr; addr < end_addr; addr += dsize)
	{
		/*
		 * If we've reached a particular interval in the test, then
		 * output a progress report.
		 */
		if ((addr & 0x1ff) == 0)
		{
			printf("\rTesting %06x ", addr);
			fflush(stdout);
		}

		/*
		 * Obtain the original data at the chosen address.
		 */
		orig_data = sram_read_sized(addr, dsize);

		/*
		 * For each bit in the chosen word/address, set that bit alone
		 * and make sure that bit alone is set when read back.
		 */
		for (bit = bitmax; bit; bit >>= 1)
		{
			if (sram_write_sized(addr, bit, dsize) == -1)
			{
				printf("Write failure at %06x, data=%08x: %s\n",
					addr, bit, dpci_last_os_error_string());
			}
			readback = sram_read_sized(addr, dsize);
			if (readback != bit)
			{
				printf("Read-back failure at %06x "
					"wanted=%08x got=%08x\n",
					addr, bit, readback);
				break;
			}
		}

		/*
		 * Restore the original data to the chosen address.
		 */
		if (sram_write_sized(addr, orig_data, dsize) == -1)
		{
			printf("write/restore failure at %06x, data=%08x: %s\n",
				addr, orig_data, dpci_last_os_error_string());
		}
	}

	/*
	 * Final status report.
	 */
	printf("\r%d bytes tested.\n", end_addr - start_addr);
}


/*******************************************************************************
 *
 * Function:	main
 *
 * Parameters:  none.
 *
 * Returns:	 nothing.
 *
 * Description: Function for handling user selection of tests.
 *
 *			  No command line parameters are parsed so presently no automated
 *			  testing is possible.
 *
 ******************************************************************************/
int main(void) 
{
	unsigned int addr = 0;
	unsigned int value = 0;
	char line[256] = "";
	int cmd = 0;
	unsigned int val32;
	unsigned short val16;
	unsigned char val8;

	menu();
	while (1)
	{
		printf("Please enter an option (0 for menu): ");
		cmd = 0;
		if(fgets(line,sizeof(line),stdin) == NULL)
		{
			break;
		}
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		sscanf(line,"%d", &cmd);

		switch(cmd) 
		{
			case SRAM_MENU:
				menu();
				break;

			case SRAM_BITWALKTEST:
				bitwalktest();
				break;

			case SRAM_ADDRESSTEST:
				addresstest();
				break;

			case SRAM_SIZE:
				value = dpci_sram_size();
				if(value == -1)
				{
					printf("Could not obtain the SRAM size"
						": %s.\n",dpci_last_os_error_string());
					break;
				}
				printf("The size of SRAM is %u.\n",value);
				break;

			case SRAM_WRITE_BYTE:
				printf("Enter offset to write to (Hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				printf("Enter data to write (Hex):");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &value);
	 	
				if (dpci_sram_write8(addr,(unsigned char)value) == -1)
				{
					perror("Can't write byte");
				}
				else
				{
					printf("Written 0x%02x to address 0x%08x\n", value, addr);
				}
	 			break;
			
			case SRAM_WRITE_WORD:
				printf("Enter address to write to (Hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				printf("Enter data to write (Hex):");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &value);
	 	
				if (dpci_sram_write16(addr,(unsigned short)value) == -1)
				{
					perror("Can't write byte");
				}
				else
				{
					printf("Written 0x%04x to address 0x%08x\n", value, addr);
				}
	 			break;
			
			case SRAM_WRITE_DWORD:
				printf("Enter address to write to (Hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				printf("Enter data to write (Hex):");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &value);
	 	
				if (dpci_sram_write32(addr,value) == -1)
				{
					perror("Can't write byte");
				}
				else
				{
					printf("Written 0x%08x to address 0x%08x\n", value, addr);
				}
	 			break;
			
			case SRAM_READ_BYTE:
				printf("Enter address to read from (in hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				if (dpci_sram_read8(addr, &val8) == -1)
				{
					perror("Can't read byte");
				}
				else
				{
					printf("Read 0x%02x from offset 0x%08x\n", val8, addr);
				}
				break;	

			case SRAM_READ_WORD:
				printf("Enter address to read from (in hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				if (dpci_sram_read16(addr, &val16) == -1)
				{
					perror("Can't read word");
				}
				else
				{
					printf("Read 0x%04x from offset 0x%08x\n", val16, addr);
				}
				break;	

			case SRAM_READ_DWORD:
				printf("Enter address to read from (in hex): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				if (dpci_sram_read32(addr, &val32) == -1)
				{
					perror("Can't read dword");
				}
				else
				{
					printf("Read 0x%08x from offset 0x%08x\n", val32, addr);
				}
				break;

			case SRAM_CLEAR:
				{
					unsigned char *zeromem;
					int size;
					int ret;
	
					size = dpci_sram_size();
					printf("Clearing SRAM (%d bytes)... ", size);
					fflush(stdout);
					zeromem = (unsigned char *)malloc(size);
					if (!zeromem)
					{
						fprintf(stderr,
								"Couldn't allocate %d bytes.\n",
								size);
						break;
					}
					memset(zeromem, 0, size);
					ret = dpci_sram_write(0, zeromem, size);
					if (ret == -1)
					{
						fprintf(stderr,
								"\nError writing block to SRAM: %s\n",
								dpci_last_os_error_string());
					}
					else if (ret == size)
					{
						printf("done.\n");
					}
					else
					{
						fprintf(stderr,
								"\nError writing block to SRAM - only %d bytes written\n",
								ret);
					}
					free(zeromem);
				}
				break;

			case SRAM_DUMP:
				{
#define BYTES_PER_LINE	16
					unsigned char sram[16];
					int bytes = 0;
					int got;

					printf("Enter address to dump from (in hex): ");
					fgets(line, sizeof(line),stdin);
					sscanf(line,"%x", &addr);
					printf("Enter # bytes to dump (in hex): ");
					fgets(line, sizeof(line),stdin);
					sscanf(line,"%x", &bytes);

					if (bytes <= 0)
					{
						break;
					}
					do {
						int i;
						int chunk;

						chunk = bytes;
						if (chunk > BYTES_PER_LINE)
							chunk = BYTES_PER_LINE;
						got = dpci_sram_read(addr, sram, chunk);
						if (got == -1)
						{
							fprintf(stderr,
									"Error reading block from SRAM at address 0x%x: %s\n",
									addr, dpci_last_os_error_string());
							break;
						}
						if (got == 0)
						{
							break;
						}
						printf("%06x: ", addr);
						for (i = 0; i < BYTES_PER_LINE; i++)
						{
							if (i >= got)
								printf("   ");
							else
								printf("%02x ", sram[i]);
						}
						printf("  ");
						for (i = 0; i < got; i++)
						{
							printf("%c", isprint(sram[i]) ? sram[i] : '.');
						}
						printf("\n");
						addr += BYTES_PER_LINE;
						bytes -= BYTES_PER_LINE;
					} while (bytes > 0 && got == BYTES_PER_LINE);
				}
				break;

			case SRAM_WRITE_STRING:
				{
					int ret;
					int size;

					printf("Enter address to write string to (in hex): ");
					fgets(line, sizeof(line),stdin);
					sscanf(line,"%x", &addr);
					printf("Enter text string: ");
					fgets(line, sizeof(line),stdin);
					size = strlen(line);
					line[strlen(line) - 1] = 0;

					if (size == 1)
					{
						break;
					}
					--size;
					ret = dpci_sram_write(addr, line, size);
					if (ret == -1)
					{
						fprintf(stderr,
								"\nError writing string to SRAM: %s\n",
								dpci_last_os_error_string());
					}
					else if (ret < size)
					{
						fprintf(stderr,
								"\nError writing string to SRAM - only %d bytes written\n",
								ret);
					}
				}
				break;		
			case SRAM_DIRECT_DUMP:
				{
					unsigned char *cp;
					int bytes = 0;
					unsigned int sram_size;

					/*
					 * Map SRAM and access directly. This
					 * function should behave and perform
					 * in an identical manner to the
					 * SRAM_DUMP function.
					 */
					printf("Enter start address (in hex): ");
					fflush(stdout);
					if(fgets(line, sizeof(line),stdin) == NULL)
					{
						break;
					}
					sscanf(line,"%x", &addr);
					printf("Enter # bytes to dump (in hex): ");
					fflush(stdout);
					if(fgets(line, sizeof(line),stdin) == NULL)
					{
						break;
					}
					sscanf(line,"%x", &bytes);

					sram_size = dpci_sram_size();


					/*
					 * Check bounds
					 */
					if(addr >= sram_size)
					{
						break;
					}

					if(addr + bytes >= sram_size)
					{
						bytes = sram_size - addr;
					}


					cp = dpci_sram_map();

					if(cp != (unsigned char*)NULL)
					{
						unsigned char *buf = malloc(bytes);

						if(buf != (unsigned char*)NULL)
						{
							cp += addr;
							memcpy(buf, cp, bytes);
							cp = buf;

							do {
								int i;

								printf("%06x: ", addr);
								for (i = 0; i < BYTES_PER_LINE; i++)
								{
									if (i >= bytes)
										printf("   ");
									else
										printf("%02x ", *(cp + i));
								}
								printf("  ");
								for (i = 0; i < BYTES_PER_LINE; i++)
								{
									unsigned char ch;

									if(i >= bytes)
									{
										break;
									}
									ch = *(cp + i);
									putchar(isprint(ch) ? ch : '.');
								}
								putchar('\n');
								cp += BYTES_PER_LINE;
								bytes -= BYTES_PER_LINE;
								addr += BYTES_PER_LINE;
							} while (bytes > 0);
						}
						dpci_sram_unmap();
					}
				}

		}/*end switch(cmd)*/
   }/*while (cmd != QUIT)*/
   return 0;
}
