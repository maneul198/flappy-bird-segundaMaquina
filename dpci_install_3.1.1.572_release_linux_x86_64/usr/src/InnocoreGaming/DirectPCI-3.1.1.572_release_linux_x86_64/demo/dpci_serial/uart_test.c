/******************************************************************************
 *
 * $Id: uart_test.c 9964 2013-06-17 14:27:28Z aidan $
 *
 * Copyright 2005-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * Test capabilities of the UARTs.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sched.h>
#include <fcntl.h>
#include <termios.h>

#define FLAG_SLOW	0x01
#define FLAG_PUTC_TX	0x02
#define FLAG_PUTC_RX	0x04
#define FLAG_INTLOOP	0x08
#define FLAG_READER	0x10
#define FLAG_WRITER	0x20

#define PROGRESS_REPORTS_DEFAULT	4096
/*
 * Structure used to pass task configuration to individual reader/writer tasks.
 */
struct uart_args
{
	char	ua_fd;		/* File descriptor for port */
	char	*ua_name;	/* name of port for messages */
	int	ua_size;	/* size of data set */
	int	ua_count;	/* count of packets to send or recieve */
	int	ua_flags;	/* other flags for operation */
	int	ua_progress;	/* how often to print progress messages */
};

/*
 * Simple table matching baud rates to termio baud-rate symbols.
 */
struct baud_rate
{
	int br_rate;
	int br_symbol;
} baud_rate_table[] = {
	{50, B50},
	{75, B75},
	{110, B110},
	{134, B134},
	{150, B150},
	{200, B200},
	{300, B300},
	{600, B600},
	{1200, B1200},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
	{19200, B19200},
	{38400, B38400},
	{57600, B57600},
	{115200, B115200},
	{230400, B230400},
	{0,0}
};

static char *argv0;


static int apply_commspec(int fd, const char *commspec, int size, const char *uart)
{
	int i;
	struct termios termio;
	int baud = 9600, csize = 8, nstop = 1;
	char parity = 'n';
	char flowctrl = 'l';
	int ret = -1;

	sscanf(commspec,"%d,%d,%c,%d,%c", &baud, &csize, &parity, &nstop, &flowctrl);

	/*
	 * Drain the existing terminal output/input so we don't get any garbage
	 */
	tcflush(fd, TCIOFLUSH);

	tcgetattr(fd, &termio);
	cfmakeraw(&termio);
	termio.c_cflag &= ~(CSIZE|PARENB|PARODD|CMSPAR);

	switch (csize)
	{
	case 5:
		termio.c_cflag |= CS5;
		break;
	case 6:
		termio.c_cflag |= CS6;
		break;
	case 7:
		termio.c_cflag |= CS7;
		break;
	case 8:
		termio.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "%d-bit character size not supported", csize);
		close(fd);
		goto failed;
	}

	switch (parity)
	{
	case 'n':
		break;
	case 'e':
		termio.c_cflag |= PARENB;
		break;
	case 'o':
		termio.c_cflag |= PARENB | PARODD;
		break;
	case '1':
	case 'm':
		termio.c_cflag |= CMSPAR | PARENB | PARODD;
		break;
	case '0':
	case 's':
		termio.c_cflag |= CMSPAR | PARENB ;
		break;
	default:
		fprintf(stderr, "Parity mode '%c' not supported", parity);
		close(fd);
		goto failed;
	}

	switch (nstop)
	{
	case 1:
		termio.c_cflag &= ~(CSTOPB);
		break;
	case 2:
		termio.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "%d stop bits not supported", nstop);
		close(fd);
		goto failed;
	}

	switch (flowctrl)
	{
	case 'm': /* modem: dtr/dsr, rts/cts & ri */
		termio.c_cflag |= HUPCL;
		termio.c_cflag &= ~(CLOCAL | CRTSCTS);
		break;
	case 'c': /* rts->cts */
		termio.c_cflag |= CRTSCTS;
		termio.c_cflag &= ~(CLOCAL | HUPCL);
		break;
	case 'l': /* local, i.e. none */
		termio.c_cflag |= CLOCAL;
		termio.c_cflag &= ~(CRTSCTS | HUPCL);
		break;
	default:
		fprintf(stderr,
				"Flow-control mode %c not supported",
				flowctrl);
		close(fd);
		goto failed;
	}


	/*
	 * Find out which baud rate symbol to use with cfsetospeed etc.
	 */
	for (i=0; baud_rate_table[i].br_rate; i++)
	{
		if (baud_rate_table[i].br_rate == baud)
		{
			cfsetospeed(&termio, baud_rate_table[i].br_symbol);
			break;
		}
	}
	if (baud_rate_table[i].br_rate == 0)
	{
		printf("Baud rate %d is not supported for %s\n", baud, uart);
		goto failed;
	}

	/*
	 * Set the minimum recieve packet size to the full packet size and
	 * set a time out of 5s for read operations.
	 */
	termio.c_cc[VMIN] = size;
	termio.c_cc[VTIME] = 50;

	/*
	 * Install the new terminal settings.
	 */
	if (tcsetattr(fd, TCSANOW, &termio) == -1)
	{
		printf("uart_test: tcsetaddr failed for %s: %s\n",
			uart, strerror(errno));
		goto failed;
	}

#if 0
	/*
	 * Set or clear internal loop-back mode bit.
	 */
	loopmode = TIOCM_LOOP;
	if (flags & FLAG_INTLOOP)
		res = ioctl(fd, TIOCMBIS, &loopmode);
	else
		res = ioctl(fd, TIOCMBIC, &loopmode);
	if (res == -1)
	{
		printf("uart_test: can't set loopback mode for \"%s\": %s\n",
			uart, strerror(errno));
		goto failed;
	}
#endif
	ret = 0;
failed:
	return ret;
}


/*******************************************************************************
 *
 * Function:    uart_rx_task
 * 
 * Parameters:  uap - arguments defining exact behaviour of task
 *
 * Returns:     nothing - exits
 *
 * Description: continuously reads data from the serial port (uap->ua_fd) whole
 *		packets of data of size ua_size.  Does simple analysis of errors
 *		and attempts to resynchronize with the source stream.
 *
 ******************************************************************************/
static void uart_rx_task(struct uart_args *uap)
{
	int got, bytes, loops, consec, errors, finished;
	unsigned char *data;
	unsigned int i, mod;

	if ((data = malloc(uap->ua_size)) == NULL)
	{
		printf("%s,rx: can't alloc buffer memory.\n",
			uap->ua_name);
		exit(1);
	}
	printf("%s,rx: started\n", uap->ua_name);
	loops = 0;
	consec = 0;
	mod = 0;
	errors = 0;
	bytes = 0;
	finished = 0;
	while (!finished)
	{
		/*
		 * Blank out data buffer to avoid any possibility of
		 * accidentally comparing stale data.
		 */
		bzero((void *)data, uap->ua_size);

		/*
		 * Get the data - complain on errors as we shouldn't get any.
		 */
		got = read(uap->ua_fd, data, uap->ua_size);
		if (got == -1)
		{
			fprintf(stderr, "%s,rx: error reading from port: %s\n",
				uap->ua_name, strerror(errno));
			continue;
		}
		bytes += got;
		if (uap->ua_flags & FLAG_PUTC_RX)
		{
			printf("R");
			fflush(stdout);
		}

		/*
		 * Do a byte-for-byte comparision now.
		 */
		for (i = 0; i < got; i++)
		{
			if (data[i] != (mod & 0xff))
			{
				consec++;
				errors++;
				printf("%s,rx: failed after %d bytes: "
					"byte#%d of %d expected %d, got %d\n",
					uap->ua_name,
					bytes, i, got, mod, data[i]);
				if (consec >= 2)
					mod = (data[i] + 1) % uap->ua_size;
				else
					mod = (mod + 1) % uap->ua_size;
			}
			else
			{
				if (consec)
					printf("%s,rx: okay after %d bytes: "
						"byte#%d of %d "
						"expected %d, got %d\n",
						uap->ua_name,
						bytes, i, got, mod, data[i]);
				mod = (mod + 1) % uap->ua_size;
				consec = 0;
			}
			if (data[i] == uap->ua_size - 1) /* start of packet */
			{
				loops++;
				if (loops == 1)
				{
					printf("%s,rx: got first packet\n",
						uap->ua_name);
				}
				else if ((loops % uap->ua_progress) == 0)
				{
					printf("%s,rx: %d packets "
						"(%d bytes), %d errors\n",
						uap->ua_name,
						loops,
						bytes,
						errors);
				}
				if (loops == uap->ua_count)
					finished++;
			}
		}
	}
	printf("%s,rx: finished - %d packets recevied.\n", uap->ua_name, loops);
	exit(0);
}


/*******************************************************************************
 *
 * Function:    uart_tx_task
 * 
 * Parameters:  uap - arguments defining exact behaviour of task
 *
 * Returns:     nothing - exits
 *
 * Description: continuously writes data to the serial port (uap->ua_fd) whole
 *		packets of data of size ua_size.  May sleep between writes if
 *		the configuration says so.
 *
 ******************************************************************************/
static void uart_tx_task(struct uart_args *uap)
{
	unsigned char *data;
	int i;
	int count = 0, bytes = 0;

	if ((data = malloc(uap->ua_size)) == NULL)
	{
		printf("%s,tx: can't alloc buffer memory.\n",
			uap->ua_name);
		exit(1);
	}
	for (i = 0; i < uap->ua_size; i++)
		data[i] = i;

	printf("%s,tx: started\n", uap->ua_name);
	sleep(2);

	/*
	 * Go around until the counter gets to zero.
	 */
	do {
		if (uap->ua_flags & FLAG_SLOW)
			sleep(1);
		bytes += write(uap->ua_fd, data, uap->ua_size);
		count++;
		if (uap->ua_flags & FLAG_PUTC_TX)
		{
			printf("t");
			fflush(stdout);
			if (count && (count % uap->ua_progress) == 0)
			{
				printf("%s,tx: %d packets (%d bytes) sent\n",
					uap->ua_name, count, bytes);
			}
		}
	} while (--uap->ua_count);
	printf("%s,tx: finished - %d packets sent.\n", uap->ua_name, count);
	exit (0);
}


/*******************************************************************************
 *
 * Function:    uart_test
 * 
 * Parameters:  uart - the name of the uart to use.
 *		size - the size in bytes of a packet to send.
 *		baud - baud rate as a decimal (not termio B* format).
 *		flags - flags concerning how the test should run.
 *		count - number of packets to use - 0 for (virtually) continuous
 *		progress - how often (# packets) to print status report
 *
 * Returns:     nothing - exits
 *
 * Description: Opens the serial ports, sets up line characterists (speed etc.)
 *		and starts the tasks as required.
 *
 ******************************************************************************/
int uart_test(char *uart,
		int size,
		const char *commspec,
		int flags,
		int count,
		int progress)
{
	char task_name[10]; /* sizeof("uart_test") + null terminator */
	struct uart_args *rx_ua = NULL, *tx_ua = NULL;
	int fd = -1;
	int loopmode;
	pid_t rx_pid;

	if ((fd = open(uart, O_RDWR, 0)) == -1)
	{
		printf("uart_test: can't open \"%s\" for w/r access: %s\n",
			uart, strerror(errno));
		goto failed;
	}

	/*
	 * Get the terminal attributes and install 'raw' mode, i.e. no signal
	 * line, XON/XOFF (s/w handshaking) or stream processing.
	 */
	if (apply_commspec(fd, commspec, size, uart) == -1)
	{
		fprintf(stderr, "failed to set terminal characteristics.\n");
		goto failed;
	}

	/*
	 * Set the default size if none is set already.
	 */
	if ((size == 0) || (size > 255))
	{
		size = 64;
		printf("uart_test: size defaulting to %d\n", size);
	}

	printf("uart_test: using device %s, packet-size, %d speed %s\n",
		uart, size, commspec);

	/*
	 * Start a reader/receiver task if required.
	 */
	if (flags & FLAG_READER)
	{
		if ((rx_ua = malloc(sizeof (*rx_ua))) == NULL)
		{
			printf("%s: can't alloc rx-args memory.\n",
				uart);
			exit(1);
		}
		rx_ua->ua_fd = fd;
		rx_ua->ua_size = size;
		rx_ua->ua_count = count;
		rx_ua->ua_flags = flags;
		rx_ua->ua_progress = progress;
		rx_ua->ua_name = uart;
		switch ((rx_pid = fork()))
		{
		case -1:
			printf("Couldn't start rx task for %s: %s\n",
				uart, strerror(errno));
			goto failed;
			break;
		case 0:
			snprintf(task_name, 9,
				"%s,rx", strchr(&uart[1], '/')+1);
			sprintf(argv0, "%-9s", task_name);
			uart_rx_task(rx_ua);
			break;
		default:
			break;
		}
	}

	/*
	 * Start a transmitter/writer task if required.
	 */
	if (flags & FLAG_WRITER)
	{
		if ((tx_ua = malloc(sizeof (*tx_ua))) == NULL)
		{
			printf("%s: can't alloc tx-args memory.\n",
				uart);
			exit(1);
		}
		tx_ua->ua_fd = fd;
		tx_ua->ua_size = size;
		tx_ua->ua_count = count;
		tx_ua->ua_flags = flags;
		tx_ua->ua_name = uart;
		tx_ua->ua_progress = progress;
		switch (fork())
		{
		case -1:
			printf("Couldn't start tx task for %s: %s\n",
				uart, strerror(errno));
			goto failed;
			break;
		case 0:
			snprintf(task_name, 9,
				"%s,tx", strchr(&uart[1], '/')+1);
			sprintf(argv0, "%-9s", task_name);
			uart_tx_task(tx_ua);
			break;
		default:
			break;
		}
	}
	return (1);

failed:
	if(NULL != rx_ua)
	{
		free(rx_ua);
		rx_ua = NULL;
	}
	if(NULL != tx_ua)
	{
		free(tx_ua);
		tx_ua = NULL;
	}
	if (fd != -1)
	{
		loopmode = 0;
#if 0
		(void) ioctl(fd, TIOCMBIC, &loopmode);
#endif
		close(fd);
	}
	return (0);
}


/*******************************************************************************
 *
 * Function:    usage
 * 
 * Parameters:  none
 *
 * Returns:     nothing - exits and kills all children (nice).
 *
 * Description: gives a description on the standard error stream outlining how
 *		the program might be invoked.
 *
 ******************************************************************************/
static void usage(void)
{
	fprintf(stderr,
		"uart_test [args]...\n"
		"\t-x dev: run reader/writer on dev\n"
		"\t-r dev: run reader on dev\n"
		"\t-w dev: run writer on dev\n"
		"\t-s #: set data size to # bytes\n"
		"\t-n #: do only # packets of data\n"
		"\t-p #: print progress every # packets (default 4096)\n"
		"\t-b #: set data baud rate to #\n"
		"\t-c baud,databits,parity,stopbits - set line spec\n"
#if 0
		"\t-l: set internal loopback mode for the device\n"
#endif
		"\t-T: emit a 't' for every packet transmitted\n"
		"\t-R: emit a 'R' for every packet received\n"
		"\t-S: operate in slow mode - one packet per second max\n"
		);
	kill(0, SIGTERM);
	exit (255);
}


/*******************************************************************************
 *
 * Function:    main
 * 
 * Parameters:  argc - number of arguments to program including its own name
 *		argv - array of argument data
 *
 * Returns:     0 if tests were started and completed okay
 *		1 if any tests failed to be started.
 *
 * Description: parses command line arguments and starts tests accordinly.
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
	int size = 0, flags = 0, userflags = 0;
	int progress = PROGRESS_REPORTS_DEFAULT, count = 0;
	int running = 0;
	char *commspec = "9600";
	int c;

	argv0 = argv[0];

	/*
	 * Go through the command line arguments.  Use getopt() to simplify
	 * interpreting flags and checking argument counts for them.
	 */
	while ((c = getopt(argc, argv, "c:b:ln:p:r:s:w:x:TRS")) != EOF)
	{
		switch (c)
		{
		case 's': /* size of data packet */
			size = strtol(optarg, 0, 0);
			break;

		case 'b': /* baud rate for line */
		case 'c': /* baud rate for line */
			commspec = optarg;
			break;

		case 'n': /* number of packets */
			count = strtol(optarg, 0, 0);
			break;

		case 'p': /* number of packets */
			progress = strtol(optarg, 0, 0);
			break;

		case 'R': /* emit an 'R' for every packet recieved */
			userflags |= FLAG_PUTC_RX;
			break;

		case 'S': /* emit packets in 'slow' mode */
			userflags |= FLAG_SLOW;
			break;

		case 'T': /* emit a 't' for every packet transmitted */
			userflags |= FLAG_PUTC_TX;
			break;

		case 'l': /* use internal loopback if supported by h/w */
			userflags |= FLAG_INTLOOP;
			break;

		case 'x': /* run reader and writer on port */
			flags = userflags | FLAG_READER | FLAG_WRITER;
			running += 2 * uart_test(optarg, size, commspec,
							flags, count, progress);
			break;

		case 'r': /* run a receiver on the port */
			flags = userflags | FLAG_READER;
			running += uart_test(optarg, size, commspec,
						flags, count, progress);
			break;

		case 'w': /* run a writer/transmitter on the port */
			flags = userflags | FLAG_WRITER;
			running += uart_test(optarg, size, commspec,
						flags, count, progress);
			break;

		case '?':
		default:
			usage();
		}
	}

	/*
	 * Complain and exit if no tests were started.
	 */
	if (!running)
	{
		printf("No tests started!\n");
		exit (1);
	}

	/*
	 * Wait for all test processes to exit.
	 */
	printf("Press control+C do halt all tests.\n");
	do {
		int status;
		pid_t pid;

		pid = wait(&status);
		printf("Process %d exited.\n", pid);
		
		running--;
	} while (running);

	/*
	 * Exit and say everything's hunky-dory.
	 */
	exit (0);
}
