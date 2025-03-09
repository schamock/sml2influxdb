#ifndef SERIAL_H
#define SERIAL_H

int initSerial();
bool readCharacter(int serialPort, char* nextByte);

#endif