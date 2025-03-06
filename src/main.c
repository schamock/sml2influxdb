#include <stdio.h>
#include <string.h>  // memset
#include <stdint.h>  // int datatypes
#include <stdbool.h> // boot datatype
#include <stdlib.h>  // exit

#include "config.h"
#include "serial.h"
#include "sml.h"

int main() {	
  char smlString[MAX_SML_STRING] = "";
  uint16_t counter = 0;
  smlResult result = {0};
  int serialPort;
  
  serialPort = initSerial();
  
  while (1) {
    // Read next character and put it to the smlString
    smlString[counter++] = readCharacterTimeout(serialPort, SERIAL_TIMEOUT_SEC);
    
    if (isSmlStringComplete(smlString, counter)) {
      if (isSmlCrcCorrect(smlString, counter)) {
        result = extractSmlData(smlString, counter);

        printf("Manufacturer: %s\n", result.manufacturer);
        printf("Value 1.8.0: %.4f Wh\n", result.value180);
        printf("Value 2.8.0: %.4f Wh\n", result.value280);
        printf("Voltages: %.1f V | %.1f V | %.1f V\n", result.voltageL1, result.voltageL2, result.voltageL3);
        printf("Total Power: %.2f W\n", result.sumActiveInstantaneousPowerTotal);
        printf("Phase Power: %.2f W | %.2f W | %.2f W\n", result.sumActiveInstantaneousPowerL1, result.sumActiveInstantaneousPowerL2, result.sumActiveInstantaneousPowerL3);
      }
      else {
        printf("CRC nok\n");
      }
      printf("-----------\n");
      // Reset everything and start over
      counter = 0;
    }
  }
	return 0;
}
