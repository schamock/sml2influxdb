#include <stdio.h>      // (f)printf
#include <string.h>     // memset
#include <stdint.h>     // int datatypes
#include <stdbool.h>    // boot datatype
#include <stdlib.h>     // exit
#include <unistd.h>     // sleep
#include <sys/param.h>  // MIN()
#include <signal.h>     // Interrupts

#include "config.h"
#include "serial.h"
#include "sml.h"
#include "influx.h"

static volatile bool shutdownRequested = false;

void sigTermHandler(int signum) {
   fprintf(stderr, "Shutdown requested! (Signal: %d)\n", signum);
   shutdownRequested = true;
}

int main() {
  int serialPort;

  char smlString[MAX_SML_STRING] = "";
  uint16_t smlSymbolCounter = 0;
  smlResult smlData = {0};

  char influxBuffer[INFLUX_MAX_STRING_NUM * INFLUX_MAX_STRING_LENGTH];
  memset(influxBuffer,0,sizeof(influxBuffer));
  uint16_t influxStringPositionNext = 0;
  uint16_t influxStringCount = 0;

  struct sigaction action = {0};
  
  // register SIGTERM Handler. This is useful, if this program is started/stopped via Systemd.
  // In this case, we can gracefully shutdown. Meaning: The current SML message will be read completely and
  // everything will be sent to InfluxDB
  action.sa_handler = sigTermHandler;
  if (0 != sigaction(SIGTERM, &action, NULL)) {
    perror("Error setting the interrupt handler");
    exit(EXIT_FAILURE);
  }
  
  // open serial Port
  serialPort = initSerial();
  
  while (1) {
    // Read next character and put it to the smlString
    bool serialResult = readCharacter(serialPort, &smlString[smlSymbolCounter]);
    if (serialResult) {
      smlSymbolCounter++;
    }
    else if (shutdownRequested) {
      // probably readCharacter was aborted by an interrupt
      // -> just go on, we are writing saved datasets and the the programm will exit
    }
    else {
      // Timeout triggered!
      // sometimes the following errors occure in /var/log/syslog:
      //   ftdi_sio ttyUSB0: usb_serial_generic_read_bulk_callback - urb stopped: -32
      // --> try to reopen the serial port. If that fails, initSerial() should fail and
      // the program will quit. In this case systemd should restart it
      fprintf(stderr, "Serial timeout! Trying to reopen serial port.\n");
      if(0 != close(serialPort)) {
        perror("Error while closing serialPort");
      }
      serialPort = initSerial();
      fprintf(stderr, "Reopen of serial port successful. Continuing...\n");
    }
      
    // check if SML string is ready and information can be extracted
    if (isSmlStringComplete(smlString, smlSymbolCounter) && !shutdownRequested) {
      if (isSmlCrcCorrect(smlString, smlSymbolCounter)) {
        smlData = extractSmlData(smlString, smlSymbolCounter);

        uint32_t freeSpace = sizeof(influxBuffer) - influxStringPositionNext - 1;
        uint16_t charsWritten = formatSmlForInflux(&influxBuffer[influxStringPositionNext], MIN(INFLUX_MAX_STRING_LENGTH, freeSpace),
                                                    smlData.value180, smlData.value280, smlData.voltageL1, smlData.voltageL2, smlData.voltageL3,
                                                    smlData.sumActiveInstantaneousPowerTotal, smlData.sumActiveInstantaneousPowerL1,
                                                    smlData.sumActiveInstantaneousPowerL2, smlData.sumActiveInstantaneousPowerL3);
        influxStringCount++;
        influxStringPositionNext += charsWritten;
      }
      else {
        // CRC doesnt match
        // this happens from time to time. Typically there is no reason to log this
        //fprintf(stderr, "CRC nok\n");
      }
      // Reset everything and start over
      smlSymbolCounter = 0;
    }

    // enough data to send or graceful shutdown requested?
    if (influxStringCount >= INFLUX_SEND_EVERY || shutdownRequested) {
      // send dataset to InfluxDB
      bool influxResult = sendInfluxData(influxBuffer);
      // If graceful shutdown was requested, we don't care about the result and exit anyways
      if (shutdownRequested) {
        fprintf(stderr, "Last data transfer to InfluxDB finished. Result: %s. Now exiting!\n",
          shutdownRequested ? "success" : "failure");
        exit(EXIT_SUCCESS);
      }
      else if (influxResult) {
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
	return 0;
}
