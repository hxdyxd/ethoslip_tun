#include "serial.h"

/*******************************************************************************
* 名称: SerialSetRawmode
* 功能: 开启原始方式
* 形参: termios
* 返回: 无
* 说明: 原始方式(输入不装配成行，也不对特殊字符进行处理)
*******************************************************************************/
static void SerialSetRawmode(struct termios *Option)
{
    Option->c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    Option->c_oflag  &= ~OPOST;/* 原始数据 */   /*Output*/
    Option->c_oflag  &= ~(OLCUC | ONLCR | OCRNL);
    /*
INPCK Enable parity check
IGNPAR Ignore parity errors
PARMRK Mark parity errors
ISTRIP Strip parity bits
IXON Enable software flow control (outgoing)
IXOFF Enable software flow control (incoming)
IXANY Allow any character to start flow again
IGNBRK Ignore break condition
BRKINT Send a SIGINT when a break condition is detected
INLCR Map NL to CR
IGNCR Ignore CR
ICRNL Map CR to NL
IUCLC Map uppercase to lowercase
IMAXBEL Echo BEL on input line too long
     * */
    Option->c_iflag  &= ~(
    		  BRKINT /* BRKINT Send a SIGINT when a break condition is detected  */
    		| INPCK  /* INPCK Enable parity check */
			| ISTRIP /* ISTRIP Strip parity bits */
			| IUCLC  /* IUCLC Map uppercase to lowercase  */
			| IMAXBEL /*  IMAXBEL Echo BEL on input line too long*/
    		);
    Option->c_iflag  &= ~(
    		  IXON   /*IXON 使能输出软件控制*/
		    | IXOFF  /*IXOFF 使能输入软件控制*/
		    | IXANY  /*IXANY允许任何字符再次开启数据流*/);
    Option->c_iflag  &= ~(
    		  ICRNL /*ICRNL把CR(0D)映射成字符NR(0A)*/
    		| INLCR /*INLCR 把字符NL(0A)映射到CR(0D)*/
			| IGNCR /*IGNCR忽略字符CR(0D)*/
	);
}

/*******************************************************************************
* 名称: SerialSetCs8NoSpace
* 功能: 数据位8位无效验
* 形参: termios
* 返回: 无
* 说明: 在海思Hi3516C_V300上使用有问题，原因不明
*******************************************************************************/
static void SerialSetCs8NoSpace(struct termios *Option)
{
    Option->c_cflag &= ~(PARENB | PARODD);
    Option->c_cflag &= ~CSTOPB;
    Option->c_cflag &= ~CSIZE;
    Option->c_cflag |= CS8;
    Option->c_cc[VTIME] = 0;    //等待数据时间(10秒的倍数)
    Option->c_cc[VMIN] = 100;  //最少可读数据
}

/*******************************************************************************
* 名称: SerialInit
* 功能: 串口初始化
* 形参: file,串口设备路径;speed,波特率
* 返回: 无
* 说明: speed:B57600,B115200等
*******************************************************************************/
int SerialInit(const char *file,int speed)
{
    struct termios opt;
    int fd = open(file, O_RDWR | O_NOCTTY);    //阻塞读方式
    if(fd > 0)
    {
        tcgetattr(fd, &opt);
        cfsetispeed(&opt, speed);
        cfsetospeed(&opt, speed);
        SerialSetRawmode(&opt);
        SerialSetCs8NoSpace(&opt);  //在海思Hi3516C_V300上使用有问题，原因不明
        tcsetattr(fd, TCSANOW, &opt);
    }
    return fd;
}

/*******************************************************************************
* 名称: SerialReadUnblock
* 功能: 非阻塞读取串口数据
* 形参: mtime,阻塞时间(单位毫秒)
* 返回: 数据长度
* 说明: 无
*******************************************************************************/
int SerialReadUnblock(int fd,char *buf,int maxlen,int mtime)
{
    int len = 0;
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    tv.tv_sec = mtime/1000;
    tv.tv_usec = mtime%1000*1000;
    if(select(1+fd, &rfds, NULL, NULL, &tv)>0)
    {
        if (FD_ISSET(fd, &rfds))
        {
            len = read(fd, buf, (size_t)maxlen);
        }
    }
    return len;
}

/*******************************************************************************
* 名称: SerialReadBlock
* 功能: 阻塞读取串口数据
* 形参:
* 返回: 数据长度
* 说明: 无
*******************************************************************************/
int SerialReadBlock(int fd,char *buf,int maxlen)
{
    return read(fd,buf,(size_t)maxlen);
}

/*******************************************************************************
* 名称: SerialWrite
* 功能: 串口发送数据
* 形参:
* 返回:
* 说明: 无
*******************************************************************************/
int SerialWrite(int fd,const char *buf,int len)
{
    return write(fd,buf,len);
}

/*******************************************************************************
* 名称: SerialClose
* 功能: 关闭串口
* 形参:
* 返回:
* 说明: 无
*******************************************************************************/
int SerialClose(int fd)
{
    return close(fd);
}

ssize_t tread(int fd, void *buf, size_t nbytes, unsigned int timout)
{
    int nfds;
    fd_set readfds;
    struct timeval  tv;

    tv.tv_sec = timout;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    nfds = select(fd+1, &readfds, NULL, NULL, &tv);
    if (nfds <= 0) {
        if (nfds == 0)
            errno = ETIME;
        return(-1);
    }
    return(read(fd, buf, nbytes));
}

ssize_t treadn(int fd, void *buf, size_t nbytes, unsigned int timout)
{
    size_t nleft;
    ssize_t nread;

    nleft = nbytes;
    while (nleft > 0) {
        if ((nread = tread(fd, buf, nleft, timout)) < 0) {
            if (nleft == nbytes)
                return(-1); /* error, return -1 */
            else
                break;      /* error, return amount read so far */
        } else if (nread == 0) {
            break;          /* EOF */
        }
        nleft -= nread;
        buf += nread;
    }
    return(nbytes - nleft);      /* return >= 0 */
}

int SerialReadBytes(int fd, uint8_t *p, uint32_t len)
{
    int rd_n =0;
    do {
        rd_n += read(fd, p + rd_n, (size_t)1);
        if(*p != 1 && *p != 2 && *p != 3){
            return rd_n;
        }
    }while(rd_n < len);
    return rd_n;
}
