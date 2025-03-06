#define _GNU_SOURCE  // otherwise CRTSCTS is not defined

#include <termios.h> // TTY
#include <fcntl.h>   //open
#include <stdio.h>   // perror
#include <stdlib.h>  // exit
#include <unistd.h>  // read
#include <poll.h>    // poll
#include <stdint.h>  // int types

#include "config.h"
#include "serial.h"

int serialPort;
struct termios tty;

void initSerial() {
  serialPort = open(TTY_DEV, O_RDONLY | O_NOCTTY);
  if (serialPort == -1) {
    perror("Fehler beim Öffnen des seriellen Ports");
    exit(EXIT_FAILURE);
  }
  
  if (tcgetattr(serialPort, &tty) != 0) {
    perror("Fehler beim Abrufen der aktuellen Einstellungen");
    exit(EXIT_FAILURE);
  }

  // Baudrate setzen (z.B. 9600)
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);
  
  // Weitere serielle Optionen einstellen
  tty.c_cflag &= ~PARENB; // Keine Parität
  tty.c_cflag &= ~CSTOPB; // Ein Stop-Bit
  tty.c_cflag &= ~CSIZE;  // Maske für die Datenbits
  tty.c_cflag |= CS8;     // 8 Datenbits
  tty.c_cflag &= ~CRTSCTS; // Keine Hardware-Flusskontrolle
  tty.c_cflag |= CREAD | CLOCAL; // Lesen aktivieren und keine Kontrolle über das Modem
  
  // Modemsteuerleitungen deaktivieren (optional)
  tty.c_lflag &= ~ICANON; // Keine kanonische Eingabe (Zeilenpufferung deaktivieren)
  tty.c_lflag &= ~ECHO;   // Kein Echo der Eingaben
  tty.c_lflag &= ~ECHOE;  // Kein ECHO beim Löschen von Zeichen
  tty.c_lflag &= ~ISIG;   // Keine Signalzeichen (wie INT, QUIT, etc.)
  
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Keine Software-Flusskontrolle
  tty.c_iflag &= ~(ICRNL | INLCR); // Keine Umwandlung von Zeichen

  // Warten bis die Einstellungen aktiv sind
  if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
    perror("Fehler beim Setzen der Port-Einstellungen");
    exit(EXIT_FAILURE);
  }

  return;
}

char readCharacter() {
  char smlChar;
  while (1) {
    ssize_t n = read(serialPort, &smlChar, sizeof(smlChar));
    if (n > 0) {
      return smlChar;
    }
    if (n < 0) {
      perror("Fehler beim Lesen");
      exit(EXIT_FAILURE);
    }
  }
}

char readCharacterTimeout(uint8_t timeoutSec) {
  char smlChar;
  struct pollfd pollPort = {0};
  int ready;
  ssize_t n;

  pollPort.fd = serialPort;
  pollPort.events = POLLIN;
  ready = poll(&pollPort, 1, timeoutSec * 1000);

  if (ready > 0) {
    if (pollPort.revents & POLLIN) {
      // char ready
      n = read(serialPort, &smlChar, sizeof(smlChar));
      if (n > 0) {
        return smlChar;
      }
      else {
        perror("read() return value <= 0");
        exit(EXIT_FAILURE);
      }
    }
    else {
      // POLLERR, POLLHUP or PHLLNVAL
      fprintf(stderr, "poll() error event: %s%s%s // \n",
          (pollPort.revents & POLLERR)  ? "POLLERR "  : "", 
          (pollPort.revents & POLLHUP)  ? "POLLHUP "  : "", 
          (pollPort.revents & POLLNVAL) ? "POLLNVAL " : ""
        );
        exit(EXIT_FAILURE);
    }
  } else if (ready == 0) {
    // timout
    perror("poll() timeout detected");
    exit(EXIT_FAILURE);
  } else {
    // error
    perror("poll() error detected");
    exit(EXIT_FAILURE);
  }
}