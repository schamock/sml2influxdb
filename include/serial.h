#ifndef SERIAL_H
#define SERIAL_H

int initSerial();
char readCharacterTimeout(int serialPort, uint8_t timeoutSec);

#endif