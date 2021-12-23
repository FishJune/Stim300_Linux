#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "com.h"

#define FALSE -1
#define TRUE 0
#define MAX_UART 8

int speed_arr[] = { B921600, B460800, B230400, B115200, B57600, B38400, B19200, B9600, B4800,
                    B2400, B1200, B300, B38400, B19200, B9600, B4800, B2400, B1200, B300,
                  };
int name_arr[] = {921600, 460800, 230400, 115200, 57600, 38400,  19200,  9600,  4800,  2400,
                  1200,  300, 38400,  19200,  9600, 4800, 2400, 1200,  300,
                 };

void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < (int)(sizeof(speed_arr) / sizeof(int));  i++)
    {
        if (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if (status != 0)perror("tcsetattr fd1");
            return;
        }
        tcflush(fd,TCIOFLUSH);
    }
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    if(tcgetattr(fd,&options)!=0)
    {
        perror("SetupSerial 1");
        return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr,"Unsupported data size\n");
        return (FALSE);
    }
    switch (parity)
    {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 'S':
    case 's':
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr,"Unsupported parity\n");
        return (FALSE);
    }
    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr,"Unsupported stop bits\n");
        return (FALSE);
    }
    if (parity != 'n')options.c_iflag |= INPCK;
	
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;
	
    tcflush(fd,TCIFLUSH);
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}




int openIMUCom(char *dev)
{
    int fd=-1;
    fd = open(dev,O_RDWR);
    if (-1 == fd)
    {
        perror("Can't Open Serial IMUODO");
        return -1;
    }
    else if (fd>0)
    {
        set_speed(fd,115200);
    }
    if (set_Parity(fd,8,1,'N')== FALSE)
    {
        printf("Set Parity Error\n");
    }
    struct termios opt;
    tcgetattr(fd,&opt);
    opt.c_lflag &=~(ICANON|ECHO|ECHOE|ECHONL|ISIG|IEXTEN);
	opt.c_iflag &=~(ICRNL|IGNCR|IXON);/***/
    opt.c_oflag &=~(OPOST);
    if(tcsetattr(fd,TCSANOW,&opt)!=0)
        printf("error");
    return fd;
}

int Open_com6()
{
    int fd=-1;
    char *dev  = "/dev/ttyS1";
    fd = open(dev,O_RDWR);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port9");
        return -1;
    }
    else if (fd>0)
    {
        set_speed(fd,115200);
    }
    if (set_Parity(fd,8,1,'N')== FALSE)
    {
        printf("Set Parity Error\n");
    }
    return fd;
}
