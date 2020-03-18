#ifndef SERIAL_H_
#define SERIAL_H_

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

int SerialInit(const char *file,int speed);   //串口初始化
int SerialReadUnblock(int fd,char *buf,int maxlen,int mtime); //非阻塞读取串口数据
int SerialReadBlock(int fd,char *buf,int maxlen); //阻塞读取串口数据
int SerialWrite(int fd,const char *buf,int len); //串口发送数据
int SerialClose(int fd);   //关闭串口
ssize_t treadn(int fd, void *buf, size_t nbytes, unsigned int timout);
int SerialReadBytes(int fd, uint8_t *p, uint32_t len);

#endif /* SERIAL_H_ */
