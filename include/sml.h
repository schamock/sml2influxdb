#ifndef SML_H
#define SML_H

#define SML_STIFF_SIZE  1
#define SML_CRC_SIZE    2
#define SML_TRAIL_SIZE  SML_STIFF_SIZE+SML_CRC_SIZE


struct SmlAddresses {
  char manufacturer[8];
  char serverId[8];
  char value180[8];
  char value280[8];
  char value181[8];
  char value182[8];
  char activePowerTotal[8];
  char activePowerL1[8];
  char activePowerL2[8];
  char activePowerL3[8];
  char publickey[8];
  char ownersNo[8];
  char voltageL1[8];
  char voltageL2[8];
  char voltageL3[8];
};

bool checkCrc(const char *message, uint16_t messageSize);
void extractSmlData (const char *message, uint16_t messageSize);
char* getPostionOfEndString (const char *smlString, uint16_t counter);
uint8_t getLengthOfEndString();

#endif