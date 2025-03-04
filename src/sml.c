#define _GNU_SOURCE  // memmem

#include <stdint.h>  // Datatypes
#include <stdbool.h> // bool
#include <string.h>  // memmem
#include <stdio.h>   // printf
#include <math.h>    // exp10

#include "sml.h"

char smlEnd[]   = { 0x1b, 0x1b, 0x1b, 0x1b, 0x1a };
char smlStart[] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x01, 0x01, 0x01, 0x01 };

struct SmlAddresses smlAddresses = {
  .manufacturer     = { 0x77, 0x07, 0x81, 0x81, 0xc7, 0x82, 0x03, 0xff },
  .serverId         = { 0x77, 0x07, 0x01, 0x00, 0x00, 0x00, 0x09, 0xff },
  .value180         = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xff },
  .value280         = { 0x77, 0x07, 0x01, 0x00, 0x02, 0x08, 0x00, 0xff },
  .value181         = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x01, 0xff },
  .value182         = { 0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x02, 0xff },
  .activePowerTotal = { 0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xff },
  .activePowerL1    = { 0x77, 0x07, 0x01, 0x00, 0x24, 0x07, 0x00, 0xff },
  .activePowerL2    = { 0x77, 0x07, 0x01, 0x00, 0x38, 0x07, 0x00, 0xff },
  .activePowerL3    = { 0x77, 0x07, 0x01, 0x00, 0x4c, 0x07, 0x00, 0xff },
  .ownersNo         = { 0x77, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff },
  .voltageL1        = { 0x77, 0x07, 0x01, 0x00, 0x20, 0x07, 0x00, 0xff },
  .voltageL2        = { 0x77, 0x07, 0x01, 0x00, 0x34, 0x07, 0x00, 0xff },
  .voltageL3        = { 0x77, 0x07, 0x01, 0x00, 0x48, 0x07, 0x00, 0xff }
};

uint16_t calculateCrc16X25(const char *data, uint16_t messageSize) {
  uint16_t crc = 0xffff;
  for (uint16_t i = 0; i < messageSize; i++) {
    crc ^= data[i];
    for (uint8_t k = 0; k < 8; k++)
      crc = (crc & 1) != 0 ? (crc >> 1) ^ 0x8408 : crc >> 1;
  }
  return ~crc;
}

bool checkCrc(const char *message, uint16_t messageSize) {
  uint16_t checksumCalc = calculateCrc16X25(message, messageSize - SML_CRC_SIZE);
  uint16_t checksumRead = message[messageSize-1]<<8 | message[messageSize-2];
  return (checksumCalc == checksumRead) ? true : false;
}

void extractSmlData (const char *message, uint16_t messageSize) {
  char* position;
  char* pointer;
  
  char manufacturer[255] = "";
  
  // Get manufacturer (OBIS 129-129:199.130.3)
  position = memmem(message, messageSize, smlAddresses.manufacturer, sizeof(smlAddresses.manufacturer));
  
  if (NULL != position) {
    // set pointer after the match
    pointer = position + sizeof(smlAddresses.manufacturer);
    // ignore the next 4 attributes (status, valTime, unit, scaler) because they are not
    // relevant for this information
    for (uint8_t i = 0; i < 4; i++) {
      // TODO: Anzahl kann auch mehrere Bytes enhalten!!
      pointer += pointer[0];
    }
    
    memcpy(manufacturer, pointer+1, pointer[0]-1);
    printf("Hersteller: %s\n", manufacturer);
  }
}

char* getPostionOfEndString (const char *smlString, uint16_t counter) {
  return memmem(smlString, counter, smlEnd, sizeof(smlEnd));
}

uint8_t getLengthOfEndString() {
  return sizeof(smlEnd);
}