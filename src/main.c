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
    //smlString[counter] = readCharacter();
    smlString[counter] = readCharacterTimeout(SERIAL_TIMEOUT_SEC);
    counter++;
    
    //// Check if smlString is complete
    // First of all, we are searching for the end bytes "smlEnd".
    // After that the CRC is checked. Searching for the start Bytes is skipped, because
    // this is obsolete when checking the CRC value. If the CRC value matches, the result
    // will be used for further calculation
    
    // Check if SML_END was found
    char* posOfEnd = getPostionOfEndString(smlString, counter);

    if (NULL != posOfEnd) {
      // if SML_END was found, we need to wait for all CRC characters
      uint16_t offset = (uint16_t)(posOfEnd - smlString);
      
      if (SML_TRAIL_SIZE <= counter - offset - getLengthOfEndString())  {
        // now we have everything and can check the CRC
        
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
        memset(&smlString[0], 0, sizeof(smlString)); // TODO really necessary???
      }
    }
  }
	
	return 0;
}
