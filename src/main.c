#include <stdio.h>
#include <string.h>  // memset
#include <stdint.h>  // int datatypes
#include <stdbool.h> // boot datatype
#include <stdlib.h>  // exit
#include <unistd.h>  // sleep

#include "config.h"
#include "serial.h"
#include "sml.h"
#include "influx.h"

int main() {
  int serialPort;

  char smlString[MAX_SML_STRING] = "";
  uint16_t smlSymbolCounter = 0;
  smlResult smlData = {0};

  char influxBuffer[INFLUX_MAX_STRING_NUM * INFLUX_MAX_STRING_LENGTH];
  memset(influxBuffer,0,sizeof(influxBuffer));
  uint16_t influxStringPositionNext = 0;
  uint16_t influxStringCount = 0;
  
  serialPort = initSerial();
  
  while (1) {
    // Read next character and put it to the smlString
    bool serialResult = readCharacter(serialPort, &smlString[smlSymbolCounter]);
    if (serialResult)
      smlSymbolCounter++;
    else
      fprintf(stderr, "Serial timeout!!\n");
      

    if (isSmlStringComplete(smlString, smlSymbolCounter)) {
      if (isSmlCrcCorrect(smlString, smlSymbolCounter)) {
        smlData = extractSmlData(smlString, smlSymbolCounter);
        
        /*
        printf("Manufacturer: %s\n", smlData.manufacturer);
        printf("Value 1.8.0: %.4f Wh\n", smlData.value180);
        printf("Value 2.8.0: %.4f Wh\n", smlData.value280);
        printf("Voltages: %.1f V | %.1f V | %.1f V\n", smlData.voltageL1, smlData.voltageL2, smlData.voltageL3);
        printf("Total Power: %.2f W\n", smlData.sumActiveInstantaneousPowerTotal);
        printf("Phase Power: %.2f W | %.2f W | %.2f W\n", smlData.sumActiveInstantaneousPowerL1, smlData.sumActiveInstantaneousPowerL2, smlData.sumActiveInstantaneousPowerL3);
        */

        uint32_t freeSpace = sizeof(influxBuffer) - influxStringPositionNext - 1;
        uint32_t maxLength = (freeSpace > INFLUX_MAX_STRING_LENGTH) ? INFLUX_MAX_STRING_LENGTH : freeSpace;
        uint16_t charsWritten = formatSmlForInflux(&influxBuffer[influxStringPositionNext], maxLength,
                                                    smlData.value180, smlData.value280, smlData.voltageL1, smlData.voltageL2, smlData.voltageL3,
                                                    smlData.sumActiveInstantaneousPowerTotal, smlData.sumActiveInstantaneousPowerL1,
                                                    smlData.sumActiveInstantaneousPowerL2, smlData.sumActiveInstantaneousPowerL3);
        influxStringCount++;
        influxStringPositionNext += charsWritten;
        
        //printf("size of buffer: %ld\n", sizeof(influxBuffer));
        //printf("Free space: %d // max length: %d // chars written: %d // string: %d // posNext: %d\n", freeSpace, maxLength, charsWritten, influxStringCount, influxStringPositionNext);

        // enough data to send?
        if (influxStringCount >= INFLUX_SEND_EVERY) {
          // send dataset to InfluxDB
          printf("Attempt to send data (%d datasets)\n", influxStringCount);
          if (sendInfluxData(influxBuffer)) {
            // sendig was successful, deleting buffers and start over again
            memset(influxBuffer,0,sizeof(influxBuffer));
            influxStringPositionNext = 0;
            influxStringCount = 0;
          }
          else if (sizeof(influxBuffer) - influxStringPositionNext < INFLUX_MAX_STRING_LENGTH) {
            // sending failed for a while and we are now running out of space!
            // lets wait a bit and retry
            for (uint16_t i = 0; i < INFLUX_NUM_RETRIES; i++) {
              sleep(INFLUX_SEC_BETWEEN_RETRIES);
              if (sendInfluxData(influxBuffer)) {
                // sendig was successful, deleting buffers and start over again
                memset(influxBuffer,0,sizeof(influxBuffer));
                influxStringPositionNext = 0;
                influxStringCount = 0;
                break;
              }
              // seams like, we are repeatedly not able to send any data :(
              fprintf(stderr, "Unable to send data after %d retries. Now quiting!\n", INFLUX_NUM_RETRIES);
              exit(EXIT_FAILURE);
            }
          }
          else {
            // error while sending and we are not yet running out of space --> just go on
          }
        }

      }
      else {
        fprintf(stderr, "CRC nok\n");
      }
      
      // Reset everything and start over
      smlSymbolCounter = 0;
    }
  }
	return 0;
}
