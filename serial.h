#ifndef SERIAL_H
#define SERIAL_H

#define CMD_LEFT     108
#define CMD_RIGHT    114
#define CMD_STOP     111
#define CMD_STRAIGHT 115
#define CMD_BACKWARD 98
#define CMD_UTURN    117

#define VAL_DISTANCE 100
#define VAL_DONE     102

#define MSG_START 42        //ASCII code for Asterisk
#define CMD       1
#define FUNC      2

int setupSerial();
void printByte(unsigned char n);
unsigned int checksum(unsigned char *str, unsigned char length);
void buildMessage(unsigned char *msg, unsigned char cmd, unsigned char func);
void sendMessage(unsigned char *msg, unsigned char payload, int fd);
int receiveMessage(unsigned char *bufRX, int fd);

#endif

