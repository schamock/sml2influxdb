#ifndef SERIAL_H
#define SERIAL_H

void initSerial();
char readCharacter();
char readCharacterTimeout(uint8_t timeoutSec);

#endif