#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>




/***************************************************************************************************
	This app can open a serial device which is the first parameter delivered to the main function,
and send a string, then, receive it.
	version: 1.2
***************************************************************************************************/








int open_uart(char* port_name);
int UART_initialize(int fd, speed_t speed, int flow_ctrl, int data_bits, int stop_bits, int parity );



int main(int argc, char*argv[])
{
	int fd;
	char rev_buff[1023];//read
	char send_buff[1023];//write
	char flag='y';
	int rev_len;
	int send_len;
	fd= open_uart(argv[1]);
	if(fd == 0)
	{
		fprintf(stderr,"open serial terminal failed...\n");
		return -1;
	}

	tcflush(fd,TCIOFLUSH);
		
	UART_initialize(fd,B115200,0,8,1,2);
	while(flag=='y'||flag=='Y')
	{
		printf("Please type in the charactors you want to send~\t:");
		scanf("%s",send_buff);
		getchar();
		send_len=write(fd,send_buff,strlen(send_buff));
		if(send_len<0)
		{
			fprintf(stderr,"send failed!\n");
			send_buff[0]='\0';
			continue;
		}
		printf("you have sent:\t%s(%dbytes)\n",send_buff,send_len);
		send_buff[0]='\0';

		printf("start receive\n");
		
		
		rev_len=read(fd,rev_buff,1024);
		if(rev_len<0)
		{
			fprintf(stderr,"receive failed");
			rev_buff[0]='\0';
		}
		else if (rev_len==0)
		{
			fprintf(stderr,"the port doesn't received any data!\n");
		}
		
		else
		{
			printf("you have received:%s\n(%dbytes)",rev_buff,rev_len);
		}
		printf("Do you want to continue?(y/n):");
		scanf("%c",&flag);
		getchar();	
	}
	close(fd);
}


/******************************open the terminal***********************************
name:	open_uart	
input: 	char* port_name: the path to the serial device.
output:	file descripter: fd ------> the termianl device.
***********************************************************************************/
int open_uart(char* port_name)
{
	int fd;//the number standing for the uart port
	//did I open the device successfully 
	fd=open(port_name, O_RDWR|O_NOCTTY|O_NDELAY);//not block
	if(fd == -1)
	{
		printf("can't open the tty port!\n");
		return 0;
	}
	//whether the device is blocked
	if(fcntl(fd, F_SETFL, 0) <0)
	{
		printf("fcntl failed!\n");
		return(0);
	}     
	else
	{
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}
	//whether the device is a tty?
	if(isatty(fileno(stdin))==0)
	{
		printf("standard input is not a terminal device\n");
		return(0);
	}
	else
	{
		printf("the stnadard input is a  tty!\n");
		
	}
	printf("fd->tty_device=%d\n",fd);
	return fd;
}
/***************************************************************************************/



/****************************************************************************************
name:		UART_initialize
function:	set the number of bits in one data unit, the stoping bit and the 
		verifying bit
input:		fd(int): the file descripter pointing to the device
		speed(speed_t):the speed of the termianl
		flow_ctrl: RTS\CTS
		data_bits: the number of the bits in a data unit 7 or 8
		stop_bits: 
		parity:    verifying type
output:		successful: 1	unsuccessful:0


****************************************************************************************/

int UART_initialize(int fd, speed_t speed, int flow_ctrl, int data_bits, int stop_bits, int parity )
{
	struct termios options;
	if(tcgetattr(fd,&options))
	{
		perror("SetupSerial 1");
		return(0);
	}
	//set bps
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);
	
	///////?
	//修改控制模式，保证程序不会占用串口
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据
	options.c_cflag |= CREAD;

	switch(flow_ctrl)
	{

		case 0 ://不使用流控制
		options.c_cflag &= ~CRTSCTS;break;
		case 1 ://使用硬件流控制
		options.c_cflag |= CRTSCTS;break;
		case 2 ://使用软件流控制
		options.c_cflag |= IXON | IXOFF | IXANY;break;
    	}
	
	//set the number of bits
	options.c_cflag &= ~CSIZE;//reset the bits mark bits except the former ones
	switch(data_bits)
	{
		case 5: options.c_cflag |= CS5;break;
		case 6: options.c_cflag |= CS6;break;
		case 7: options.c_cflag |= CS7;break;
		case 8: options.c_cflag |= CS8;break;
		default: fprintf(stderr,"Unsupported data size\n");
			return 0;
	}	
	//parity bit
	switch(parity)
	{
		case 0:// no parity bit 
			options.c_cflag &=~PARENB;
			options.c_cflag &=~INPCK;
			break;
		case 1://odd number
			options.c_cflag |=(PARODD|PARENB);
			options.c_cflag |=INPCK;
			break;
		case 2://even number
			options.c_cflag |=(PARENB|INPCK);
			options.c_cflag &=~PARODD;
			break;
		
	} 
	switch(stop_bits)
	{
		case 1://1 stop bit
			options.c_cflag &=~CSTOPB;
			break;
		case 2://2 stop bits
			options.c_cflag |=CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return(0);
	}
	//close special output mode
	options.c_oflag &= ~OPOST;
	
	//wait time and minmum number of "bytes"
	options.c_cc[VTIME]= 1;//wait for 0.1s
	options.c_cc[VMIN]= 1;//read at least 1 byte
	//if flush, receive the data do not read
	tcflush(fd,TCIFLUSH);
	
	//set options
	if(tcsetattr(fd,TCSANOW,&options)!=0)
	{
		perror("tty set error!\n");
		return 0;
	}
	return 1;
}
/*************************************************************************************************
name: 		UART_rcv
function: 	receive the data from serial port
input:		fd:	file
		rcv_buf
		data_len
output:		successful:	1
		unsuccessful:	0
**************************************************************************************************/
/*int UART_rcv(int fd, char*rcv_buf,data_len)
{
	
}*/














