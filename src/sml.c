#define _GNU_SOURCE  // memmem

#include <stdint.h>  // Datatypes
#include <stdbool.h> // bool
#include <string.h>  // memmem
#include <stdio.h>   // printf
#include <math.h>    // exp10

#include "sml.h"

char smlEnd[]   = { 0x1b, 0x1b, 0x1b, 0x1b, 0x1a };
char smlStart[] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x01, 0x01, 0x01, 0x01 };

enum SmlType {
  SML_UNDEF = 0,
  SML_INT = 1,
  SML_UINT = 2,
  SML_BOOL = 3,
  SML_OCTET = 4
};

struct SmlAttribute {
  enum SmlType typeOfAttribute;
  union {
    uint64_t uint64;
    int64_t  int64;
    struct
    {
      char* octetPointer;
      uint16_t octedLength;
    };
  };
  //char* attributePointer;
  //uint16_t lengthOfAttribute;
  char* nextAttribute;
};

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

struct SmlAttribute getNextAttribute(char* startPointer, uint16_t maxLength) {
  struct SmlAttribute returnValue = {0};
  char* curPointer = startPointer;
  uint8_t curByte = curPointer[0];
  // the lower four bits are relevant for the length
  uint16_t lengthOfAttribute = (curByte & 0x0F);

  // determine type of attribute
  switch (curByte & 0x70) {
    case 0x00:
      returnValue.typeOfAttribute = SML_OCTET;
      break;
    case 0x40:
      returnValue.typeOfAttribute = SML_BOOL;
      break;
    case 0x50:
      returnValue.typeOfAttribute = SML_INT;
      break;
    case 0x60:
      returnValue.typeOfAttribute = SML_UINT;
      break;
    default:
      returnValue.typeOfAttribute = SML_UNDEF;
      break;
  }

  // if the first bit (match against 0x80) the next byte also contributes to the length
  // safety check: don't exeed maxLength
  while ((curByte & 0x80) != 0 && (curPointer-startPointer) < maxLength) {
    // first of all shift the existing bits to the left
    lengthOfAttribute <<= 4;
    // set the pointer to the next byte and safe that byte to curByte
    curPointer++;
    curByte = curPointer[0];
    lengthOfAttribute |= (curByte & 0x0F);
    printf("found something!!!\n");
  }
  
  printf("Current: %02X  ### Length: %d\n", curByte, lengthOfAttribute);

  if (lengthOfAttribute-1 > 0) {
    if (returnValue.typeOfAttribute == SML_OCTET) {
      printf("Content (str): %.*s\n", lengthOfAttribute-1, curPointer+1);
      returnValue.octetPointer = curPointer+1;
      returnValue.octedLength = lengthOfAttribute-1;
    }
    else if (returnValue.typeOfAttribute == SML_INT || returnValue.typeOfAttribute == SML_UINT) {
      //int64_t test = 0;
      if (returnValue.typeOfAttribute == SML_INT && curPointer[1] & 0x80)
        returnValue.int64 = -1;
      
      for (uint16_t i = 1; i<lengthOfAttribute; i++)
        returnValue.int64 = (returnValue.int64 << 8) | (uint8_t)curPointer[i];

      printf("\nContent (int): %ld\n", returnValue.int64);
    }
  }
  else
    printf("No content!\n");

  returnValue.nextAttribute = startPointer += lengthOfAttribute;
  return returnValue;
}

void extractSmlData (const char *message, uint16_t messageSize) {
  char* position;
  char* pointer;
  struct SmlAttribute retVal;
  
  char manufacturer[255] = "";
  int64_t value180 = 0;
  int8_t scaler = 0;
  
  // Get manufacturer (OBIS 129-129:199.130.3)
  position = memmem(message, messageSize, smlAddresses.manufacturer, sizeof(smlAddresses.manufacturer));
  
  if (NULL != position) {
    // set pointer after the match
    pointer = position + sizeof(smlAddresses.manufacturer);
    // 1. Field: status -> ignore it
    retVal = getNextAttribute(pointer, messageSize - (pointer - message));
    // 2. Field: valTime -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // 3. Field: unit -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // 4. Field scaler -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // 5. Field value -> this is interesting
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // copy the value to "manufacturer"
    memcpy(manufacturer, retVal.octetPointer, retVal.octedLength);    
    
    printf("Hersteller: %s\n", manufacturer);
  }

  // Get value 1.8.0 (OBIS 129-129:199.130.3)
  position = memmem(message, messageSize, smlAddresses.value180, sizeof(smlAddresses.value180));
  if (NULL != position) {
    // set pointer after the match
    pointer = position + sizeof(smlAddresses.value180);
    // 1. Field: status -> ignore it
    retVal = getNextAttribute(pointer, messageSize - (pointer - message));
    // 2. Field: valTime -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // 3. Field: unit -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    // 4. Field scaler -> ignore it
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));
    scaler = retVal.int64;
    // 5. Field value -> this is interesting
    retVal = getNextAttribute(retVal.nextAttribute, messageSize - (retVal.nextAttribute - message));

    value180 = retVal.int64 * exp10(scaler);
    printf("1.8.0 (no scal): %ld Wh\n", value180);
  }
}

char* getPostionOfEndString (const char *smlString, uint16_t counter) {
  return memmem(smlString, counter, smlEnd, sizeof(smlEnd));
}

uint8_t getLengthOfEndString() {
  return sizeof(smlEnd);
}