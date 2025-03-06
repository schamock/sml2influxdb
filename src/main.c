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
  
  initSerial();
  
  while (1) {
    // Read next character and put it to the smlString
    smlString[counter++] = readCharacterTimeout(SERIAL_TIMEOUT_SEC);
    
    if (isSmlStringComplete(smlString, counter)) {
      if (checkCrc(smlString, counter)) {
        // The CRC matches and we can process the result
        printf("CRC ok\n");
        extractSmlData(smlString, counter);
      }
      else {
        // The CRC does not match and the result is not usable
        printf("CRC nok\n");
      }
      // Reset everything and start over
      counter = 0;
    }
  }
	
	return 0;
}
