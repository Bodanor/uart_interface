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

#include "uart.h"

#include <stdlib.h>

void setup_terminal(void);
void *get_from_device(void *arguments);

int main(int argc, char *argv[]) {
	setup_terminal();
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

		while (1)
		{
			char c;
			while ((c = getchar()))
			{
				if (c == '\n')
					uart_write(&dev, '\r');
				else
				{
					uart_write(&dev, c);
				}
			}

			pthread_join(sniffer_send_device_thread, NULL);
		}
	}

}

void setup_terminal(void)
{
	int c;   
    static struct termios oldt, newt;

    tcgetattr( STDIN_FILENO, &oldt);
    
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);          

    tcsetattr( STDIN_FILENO, TCSANOW, &newt);      

}

void *get_from_device(void *dev_struct)
{
	while (1)
	{
		printf("%c", uart_read(dev_struct));
		fflush(stdout);
	}

}