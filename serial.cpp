#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "serial.h"

/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyUSB0"
#define BAUDRATE B9600

/**
int main(void){
  printf("Starting program\n");
  int fd = setupSerial();
  int res;
  unsigned char bufRX[255];
  int toggle = 0;
  unsigned char msgTX[10];
  while(1){
    int rx_length = receiveMessage(bufRX, fd);
    if (rx_length > 0)
      printf("Start of message\n");
    for (int i=0; i<rx_length; i++){
      printByte(bufRX[i]);
    }
    
    if (toggle){
      buildMessage(msgTX,CMD_STOP,0);
      sendMessage(msgTX,3,fd);
      toggle = 0;
    } else {
      buildMessage(msgTX,CMD_RIGHT,15);
      sendMessage(msgTX,3,fd);
      toggle = 1;
    }
    usleep(2000000);
  }
  close(fd);
  return 0;
}
*/
int receiveMessage(unsigned char *bufRX, int fd){
  if (fd != -1) {
    // Read up to 255 characters from the port if they are there
    return read(fd, (void*)bufRX, 255);		//Filestream, buffer to store in, number of bytes to read (max)
  } else {
    return -1;
  }
}

unsigned int checksum(unsigned char *str, unsigned char length){
  unsigned int sum = 0;
  for(int i=0; i<length; i++)
    sum += str[i];
  return sum;
}

void sendMessage(unsigned char *msg, unsigned char payload, int fd){
  int check;
  unsigned char chkH, chkL;
  check = checksum(msg,payload);
  chkH = (unsigned char)(check >> 8);
  chkL = (unsigned char)(check&0x00FF);
  msg[payload] = chkH;
  msg[payload + 1] = chkL;
  //for (int i=0;i<ARGS + payload + 2;i++) printByte(msg[i]);
  if (fd != -1) {
    int count = write (fd, msg, payload + 2);		//Filestream, bytes to write, number of bytes to write
    if (count < 0) {
      printf("UART TX error\n");
    } else {
      printf("Message sent---------------- cmd:%c, val%d\n", msg[CMD], msg[FUNC]);
    }
  }
}

void buildMessage(unsigned char *msg,
                  unsigned char cmd,
                  unsigned char func){
  msg[0]=MSG_START;
  msg[CMD]=cmd;
  msg[FUNC]=func;
}

void printByte(unsigned char n){
  unsigned char h, t, u, num;
  h = n / 100;
  t = (n / 10) % 10;
  u = n % 10;
  num = (h*100) + (t*10) + u;
  printf("%d\n", num);
} // Change to print characters as well as numbers??

int setupSerial(){
  //-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
	int fd = -1;

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (fd == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
  return fd;
}

