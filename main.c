/*
 * main.c
 *
 * @date 2019/08/09
 * @author Cosmin Tanislav
 * @author Cristian Fatu
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "uart.h"

#include <stdlib.h>

struct termios *setup_terminal(void);
void *get_from_device(void *arguments);


static char done = 0;
static void sigHandler(int signum)
{

	printf("Caught Cancel SIGNAL !\nStarting Cleanup process ...\n");
    done = 1;
}

int main(int argc, char *argv[]) {

	struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigHandler;
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


	struct termios *oldt = setup_terminal();
	if (argc == 1)
	{
		printf("Argument device missing !\n\nUsage : uart <devicename>\t/dev/ttyACMx\n");
		return -1;
	}
	else
	{
		printf("UART STARTED !\n");
		struct UartDevice dev;
		int rc;
		
		dev.filename = argv[1];
		dev.rate = B9600;

		rc = uart_start(&dev, false);
		if (rc)
		{
			return rc;
		}

		pthread_t sniffer_send_device_thread;
		pthread_create(&sniffer_send_device_thread, NULL, get_from_device, (void *)&dev);

		while (!done)
		{
			char c;
			while ((c = getchar()) && c != EOF)
			{
				
				if (c == '\n')
					uart_write(&dev, '\r');
				else
				{
					uart_write(&dev, c);
				}
			}

		}
		pthread_cancel(sniffer_send_device_thread);
		tcsetattr( STDIN_FILENO, TCSANOW, oldt);
	}

}

struct termios *setup_terminal(void)
{
    struct termios *oldt = malloc(sizeof(struct termios));
	if (oldt == NULL)
		return NULL;
	struct termios newt;

    tcgetattr( STDIN_FILENO, oldt);
    
	newt = *oldt;

    newt.c_lflag &= ~(ICANON | ECHOE | ECHO);          

    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	return oldt;    

}

void *get_from_device(void *dev_struct)
{
	char rc = 0;
	while ((rc =uart_read(dev_struct)) != -1)
	{
		printf("%c", rc);
		fflush(stdout);
	}

}