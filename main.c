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
#include <dirent.h>
#include "uart.h"

#include <stdlib.h>

struct termios *setup_terminal(void);
void *get_from_device(void *arguments);
void *send_to_device(void *arguments);

int main(int argc, char *argv[]) {

	DIR *d;
	struct dirent *dir;
	struct termios *oldt = setup_terminal();
	short interface_found = 0;

	if (argc == 1)
	{
		printf("Argument device missing !\n\nUsage : uart <devicename>\t/dev/ttyACMx\n\n");
		d = opendir("/dev/");
		if (d)
		{
			while ((dir = readdir(d)) != NULL && interface_found == 0)
			{
				if (strncmp(dir->d_name, "ttyACM", 6) == 0)
				{
					interface_found = 1;
				}
			}
		}
		if (interface_found)
		{
			d = opendir("/dev/");
			printf("Possible interface : \n");
			
			if (d)
			{
				while ((dir = readdir(d)) != NULL && interface_found != 0)
				{
					if (strncmp(dir->d_name, "ttyACM", 6) == 0)
					{
						printf("%s\n", dir->d_name);
					}
				}
				putchar('\n');
			}
		}
		else
			printf("No interface found !\n\n");
		tcsetattr( STDIN_FILENO, TCSANOW, oldt);
		free(oldt);
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
			tcsetattr( STDIN_FILENO, TCSANOW, oldt);
			free(oldt);
			return rc;
		}

		pthread_t sniffer_read_device_thread;
		pthread_t sniffer_send_device_thread;
		pthread_create(&sniffer_read_device_thread, NULL, get_from_device, (void *)&dev);
		pthread_create(&sniffer_send_device_thread, NULL, send_to_device, (void*)&dev);

		pthread_join(sniffer_read_device_thread, NULL);
		pthread_cancel(sniffer_send_device_thread);
		tcsetattr( STDIN_FILENO, TCSANOW, oldt);
		uart_stop(&dev);
		free(oldt);
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
	while ((rc =uart_read(dev_struct)) != 0)
	{
		printf("%c", rc);

		fflush(stdout);
	}
}
void *send_to_device(void *dev_struct)
{
	char c;
	while ((c = getchar()) && c != EOF)
	{
		
		if (c == '\n')
		{
			//uart_write(&dev, '\r');
			uart_write(dev_struct, '\n');
		}
		else
			uart_write(dev_struct, c);
	}
}