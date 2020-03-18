#ifndef __UART2_H__
#define __UART2_H__

#include <stdint.h>

void uart2_write_string(char *pstr);
int uart2_init(const char *file, uint32_t speed);
void send_char(unsigned char ch);
void send_char_do(void);
unsigned char recv_char(void);


#endif
