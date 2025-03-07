#ifndef SERIAL_H
#define SERIAL_H

int initSerial();
//char readCharacterTimeout(int serialPort, uint8_t timeoutSec);
bool readCharacter(int serialPort, char* nextByte);

#endif