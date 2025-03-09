#ifndef CONFIG_H
#define CONFIG_H

/*
You have to adjust at least these values according to your needs:
- SERIAL_TTY_DEV --> here goes your serial device, you are using to receive the SML data
InfluxDB v2 config:
- INFLUX_SERVER --> the URL of the InfluxDB v2 server. This has to include https/http in front and should not include a "/" in the end
- INFLUX_ORG --> the org, you configured within InfluxDB
- INFLUX_BUCKET --> the bucket, where you want to store the data
- INFLUX_TOKEN --> the token with write access to this bucket
- INFLUX_MEASUREMENT --> the measurement, that should be used

All other parameters should only be changed, if you know, what you are doing ;)
There are no checks for the parameters to make any sense and currently I don't plan to add any
*/

#define SERIAL_TTY_DEV      "/dev/ttyUSB0"
#define MAX_SML_STRING      2048
#define SERIAL_TIMEOUT_SEC  5

#define INFLUX_SERVER               "https://[ip/hostname]:8086"
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