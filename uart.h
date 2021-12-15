/*
 * uart.h
 *
 * @date 2019/08/09
 * @author Cosmin Tanislav
 * @author Cristian Fatu
 */

#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef SRC_UART_H_
#define SRC_UART_H_

#define DEBUG

struct UartDevice {
	char* filename;
	int rate;

	int fd;
	struct termios *tty;
};

int uart_start(struct UartDevice* dev, bool canonic);
char uart_read(struct UartDevice* dev);
int uart_write(struct UartDevice* dev, char c);
void uart_stop(struct UartDevice* dev);

#endif /* SRC_UART_H_ */
