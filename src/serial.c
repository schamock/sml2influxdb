#define _GNU_SOURCE  // otherwise CRTSCTS is not defined

#include <termios.h> // TTY
#include <fcntl.h>   // open
#include <stdio.h>   // perror
#include <stdlib.h>  // exit
#include <unistd.h>  // read
#include <stdbool.h> // boot datatype

#include "config.h"
#include "serial.h"

struct termios tty;

/*
TTY settings are inspired by this website:
https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
*/

int initSerial() {
  int serialPort = open(SERIAL_TTY_DEV, O_RDONLY | O_NOCTTY);
  if (serialPort == -1) {
    perror("Error opening the serial port!");
    exit(EXIT_FAILURE);
  }
  
  if (tcgetattr(serialPort, &tty) != 0) {
    perror("Error retrieving current tty settings!");
    exit(EXIT_FAILURE);
  }

  cfsetspeed(&tty, B9600);
  
  tty.c_cflag &= ~PARENB;         // no parity
  tty.c_cflag &= ~CSTOPB;         // one stop bit
  tty.c_cflag &= ~CSIZE;          // clear number of databits ...
  tty.c_cflag |= CS8;             // ... and set it to 8 databits
  tty.c_cflag &= ~CRTSCTS;        // disable HW-flowcontrol
  tty.c_cflag |= CREAD | CLOCAL;  // activate reading and disable modem specific stuff
  
  tty.c_lflag &= ~ICANON; // disable cononcial mode
  tty.c_lflag &= ~ECHO;   // disable echo
  tty.c_lflag &= ~ECHOE;  // disable echo erasure
  tty.c_lflag &= ~ECHONL; // disable new line echo
  tty.c_lflag &= ~ISIG;   // no interpretation of special chars (like INTR, QUIT, etc.)
  
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                           // no SW-flowcontrol
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);  // Disable any special handling of received bytes

  // VTIME defines the Timeout waiting for the next symbol. 0.1s accuracy
  tty.c_cc[VTIME] = SERIAL_TIMEOUT_SEC * 10;
  tty.c_cc[VMIN] = 0;

  // apply settings
  if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
    perror("Fehler beim Setzen der Port-Einstellungen");
    exit(EXIT_FAILURE);
  }

  return serialPort;
}

bool readCharacter(int serialPort, char* nextByte) {
  ssize_t n = read(serialPort, nextByte, 1);
  if (n > 0) {
    return true;
  }
  else if (n < 0) {
    perror("Fehler beim Lesen");
    close(serialPort);
    exit(EXIT_FAILURE);
  }
  else {
    return false;
  }
}