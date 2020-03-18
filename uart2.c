#include "serial.h"
#include <stdint.h>

int fd;

#define QUEUE_MAX_NUM   (4000)
const speed_t gs_usart_rate[] = {
		B1800, B2400, B4800, B9600,
		B19200, B38400, B57600,
		B115200, B230400, B460800, B500000, B576000, B921600,
		B1000000, B1152000, B1500000, B2000000, B2500000};

const uint32_t gs_usart_rate_val[] = {
		1800, 2400, 4800, 9600,
		19200, 38400, 57600,
		115200, 230400, 460800, 500000, 576000, 921600,
		1000000, 1152000, 1500000, 2000000, 2500000};


#if 1
static unsigned char gs_tx_buff[QUEUE_MAX_NUM];
static unsigned short gs_tx_ptr = 0;
void send_char(unsigned char ch)
{
  	if(gs_tx_ptr < QUEUE_MAX_NUM) {
  		gs_tx_buff[gs_tx_ptr++] = ch;
  	}
}

void send_char_do(void)
{
	write(fd, gs_tx_buff, gs_tx_ptr);
	gs_tx_ptr = 0;
}
#else
void send_char(unsigned char ch)
{
	int ret;
	do {
		ret = write(fd, &ch, 1);
	}while(ret != 1);
}

void send_char_do(void)
{
}
#endif

#if 1
static unsigned char gs_rx_buff[QUEUE_MAX_NUM];
static unsigned short gs_rx_ptr = 0;
static int rx_buf_len = 0;

unsigned char recv_char(void)
{
	if(rx_buf_len == 0 || gs_rx_ptr == rx_buf_len) {
		rx_buf_len = read(fd, gs_rx_buff, (size_t)QUEUE_MAX_NUM);
		gs_rx_ptr = 0;
#ifdef DEBUG
		printf("rx_buf_len = %d\n", rx_buf_len);
#endif
	}

	return gs_rx_buff[gs_rx_ptr++];
}
#else
unsigned char recv_char(void)
{
	unsigned char ch;
	read(fd, &ch, (size_t)1);
	return ch;
}
#endif


void uart2_write_string(char *pstr)
{
    unsigned int i=0;

    while (*(pstr+i) != 0) {
    	send_char( *(pstr+i));
        i++;
    }
}

int uart2_init(const char *file, uint32_t speed)
{
	speed_t uart_speed = B0;
    // uart test
	for(int i=0; i<sizeof(gs_usart_rate_val)/sizeof(gs_usart_rate_val[0]); i++) {
		if(gs_usart_rate_val[i] == speed) {
			uart_speed = gs_usart_rate[i];
			break;
		}
	}

	if(uart_speed == B0) {
		return -1;
	}

	fd = SerialInit(file, uart_speed);
	if(fd < 0) {
		return -1;
	}
	return 0;
}

