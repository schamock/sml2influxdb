#define _GNU_SOURCE  // memmem

#include <stdint.h>  // int Datatypes
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
  .publickey        = { 0x77, 0x07, 0x81, 0x81, 0xc7, 0x82, 0x05, 0xff },
  .ownersNo         = { 0x77, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff },
  .voltageL1        = { 0x77, 0x07, 0x01, 0x00, 0x20, 0x07, 0x00, 0xff },
  .voltageL2        = { 0x77, 0x07, 0x01, 0x00, 0x34, 0x07, 0x00, 0xff },
  .voltageL3        = { 0x77, 0x07, 0x01, 0x00, 0x48, 0x07, 0x00, 0xff }
};

enum SmlType {
  SML_UNDEF = 0,
  SML_INT = 1,
  SML_UINT = 2,
  SML_BOOL = 3,
  SML_OCTET = 4
};

typedef struct {
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
  char* nextAttribute;
} smlAttribute;

typedef struct {
  bool         isSet;
  smlAttribute objName;
  smlAttribute status;
  smlAttribute valTime;
  smlAttribute unit;
  smlAttribute scaler;
  smlAttribute value;
  smlAttribute valueSignature;
} smlListEntry;

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

smlAttribute getNextSmlAttribute(char* startPointer, uint16_t maxLength) {
  smlAttribute returnValue = {0};
  char* curPointer = startPointer;
  uint8_t curByte = curPointer[0];
  uint8_t dataOffset = 1;
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
    dataOffset++;
    //printf("found something!!!\n");
  }
  
  //printf("Current: %02X  ### Length: %d\n", curByte, lengthOfAttribute);

  if (lengthOfAttribute-1 > 0) {
    if (returnValue.typeOfAttribute == SML_OCTET) {
      //printf("Content (str): %.*s\n", lengthOfAttribute-1, curPointer+1);
      returnValue.octetPointer = curPointer+1;
      returnValue.octedLength = lengthOfAttribute-dataOffset;
    }
    else if (returnValue.typeOfAttribute == SML_INT || returnValue.typeOfAttribute == SML_UINT) {
      // same handling for int and uint
      // if we see a negativ int, the value will be initialized with 0xFFffFFffFFffFFff (== -1)
      // to take into considaration, that int8, int16, int32 could also be sent via SML 
      if (returnValue.typeOfAttribute == SML_INT && curPointer[1] & 0x80)
        returnValue.int64 = -1;
      
      // every byte needs to be added to the total value
      for (uint16_t i = 1; i<lengthOfAttribute; i++)
        returnValue.int64 = (returnValue.int64 << 8) | (uint8_t)curPointer[i];

      //printf("\nContent (int): %ld\n", returnValue.int64);
    }
  }
  /*
  else
    printf("No content!\n");
  */

  returnValue.nextAttribute = startPointer += lengthOfAttribute;
  return returnValue;
}

smlListEntry getSmlListEntry(const char *message, uint16_t messageSize, const char* entryName) {
  char* currentPosition;
  smlListEntry listEntry = {0};
  smlAttribute curAtt = {0};

  currentPosition = memmem(message, messageSize, entryName, sizeof(entryName));
  if (NULL != currentPosition) {
    // Skip 0x77 symbol -> Pointer + 1
    curAtt.nextAttribute = currentPosition + 1;
    curAtt = listEntry.objName        = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.status         = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.valTime        = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.unit           = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.scaler         = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.value          = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    curAtt = listEntry.valueSignature = getNextSmlAttribute(curAtt.nextAttribute, messageSize - (curAtt.nextAttribute - message));
    listEntry.isSet = true;
  }
  return listEntry;
}

void extractSmlData (const char *message, uint16_t messageSize) {
  smlListEntry entry;

  entry = getSmlListEntry(message, messageSize, smlAddresses.manufacturer);
  if (entry.isSet)
    printf("Manufacturer: %.*s\n", entry.value.octedLength, entry.value.octetPointer);

  entry = getSmlListEntry(message, messageSize, smlAddresses.value180);
  if (entry.isSet)
    printf("Value 1.8.0: %.4f Wh\n", entry.value.int64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.value280);
  if (entry.isSet)
    printf("Value 2.8.0: %.4f Wh\n", entry.value.int64 * exp10(entry.scaler.int64));

  entry = getSmlListEntry(message, messageSize, smlAddresses.voltageL1);
  if (entry.isSet)
    printf("Voltages: %.1f V | ", entry.value.uint64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.voltageL2);
  if (entry.isSet)
    printf("%.1f V | ", entry.value.uint64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.voltageL3);
  if (entry.isSet)
    printf("%.1f V\n", entry.value.uint64 * exp10(entry.scaler.int64));

  entry = getSmlListEntry(message, messageSize, smlAddresses.activePowerTotal);
  if (entry.isSet)
    printf("Total Power: %.2f W\n", entry.value.int64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.activePowerL1);
  if (entry.isSet)
    printf("Phase Power: %.2f W | ", entry.value.int64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.activePowerL2);
  if (entry.isSet)
    printf("%.2f W | ", entry.value.int64 * exp10(entry.scaler.int64));
  entry = getSmlListEntry(message, messageSize, smlAddresses.activePowerL3);
  if (entry.isSet)
    printf("%.2f W\n", entry.value.int64 * exp10(entry.scaler.int64));

  entry = getSmlListEntry(message, messageSize, smlAddresses.publickey);
  if (entry.isSet) {
    printf("Publickey (Length: %d):", entry.value.octedLength);
    for (uint8_t i = 0; i < entry.value.octedLength; i++) {
      if (i%10 == 0) printf("\n");
      printf("0x%02X ", entry.value.octetPointer[i]);
    }
    printf("\n");
  }

  printf("-----------\n");
}

char* getPostionOfEndString (const char *smlString, uint16_t counter) {
  return memmem(smlString, counter, smlEnd, sizeof(smlEnd));
}

uint8_t getLengthOfEndString() {
  return sizeof(smlEnd);
}