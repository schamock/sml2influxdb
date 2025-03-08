#ifndef CONFIG_H
#define CONFIG_H

#define SERIAL_TTY_DEV      "/dev/ttyUSB0"
#define MAX_SML_STRING      2048
#define SERIAL_TIMEOUT_SEC  5

#define INFLUX_ORG                  "THE_ORG"
#define INFLUX_BUCKET               "THE_BUCKET"
#define INFLUX_TOKEN                "THE_TOKEN"
#define INFLUX_MEASUREMENT          "THE_MEASUREMENT"
#define INFLUX_MAX_STRING_LENGTH    1024
// wait for aprox 15 minutes (assumption: 1 message every second: 15*60=900)
#define INFLUX_MAX_STRING_NUM       900
#define INFLUX_SEND_EVERY           60
#define INFLUX_NUM_RETRIES          5
#define INFLUX_SEC_BETWEEN_RETRIES  30

#endif