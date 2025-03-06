#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <curl/curl.h> // sudo apt install libcurl4-openssl-dev
#include "config.h"

#define INFLUXDB_URL  "https://influx.ws3:8086/api/v2/write?org=" INFLUX_ORG "&bucket=" INFLUX_BUCKET "&precision=s"
#define LINE_PROTOCOL INFLUX_MEASUREMENT ",location=office value=23.5"

/*
 Curl statement:
 curl --request POST \
        "http://localhost:8086/api/v2/write?org=YOUR_ORG&bucket=YOUR_BUCKET&precision=ns" \
        --header "Authorization: Token YOUR_API_TOKEN" \
        --header "Content-Type: text/plain; charset=utf-8" \
        --header "Accept: application/json" \
        --data-binary '
        airSensors,sensor_id=TLM0201 temperature=73.97038159354763,humidity=35.23103248356096,co=0.48445310567793615 1630424257000000000
        airSensors,sensor_id=TLM0202 temperature=75.30007505999716,humidity=35.651929918691714,co=0.5141876544505826 1630424257000000000
        '
*/

void formatSmlForInflux(char* influxString,
                        uint16_t stringLength,
                        double value180,
                        double value280,
                        double voltageL1,
                        double voltageL2,
                        double voltageL3,
                        double sumActiveInstantaneousPowerTotal,
                        double sumActiveInstantaneousPowerL1,
                        double sumActiveInstantaneousPowerL2,
                        double sumActiveInstantaneousPowerL3 ) {
    snprintf(influxString, stringLength-1, INFLUX_MEASUREMENT " smlValue180=%.4f,smlValue280=%.4f,"
                      "sumActiveInstantaneousPowerTotal=%.2f,sumActiveInstantaneousPowerL1=%.2f," 
                      "sumActiveInstantaneousPowerL2=%.2f,sumActiveInstantaneousPowerL3=%.2f,"
                      "smlVoltageL1=%.1f,smlVoltageL2=%.1f,smlVoltageL3=%.1f %ld",
                    value180, value280, sumActiveInstantaneousPowerTotal, sumActiveInstantaneousPowerL1,
                    sumActiveInstantaneousPowerL2, sumActiveInstantaneousPowerL3, voltageL1, voltageL2, voltageL3,
                    time(NULL));
}

int test(void) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *headerList = {0};

  curl_global_init(CURL_GLOBAL_SSL);
  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, INFLUXDB_URL);

    headerList = curl_slist_append(headerList, "Authorization: Token " INFLUX_TOKEN);
    headerList = curl_slist_append(headerList, "Content-Type: text/plain; charset=utf-8");
    headerList = curl_slist_append(headerList, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, LINE_PROTOCOL);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "Error sending to InfluxDB: %s\n", curl_easy_strerror(res));
    } else {
      printf("Data successfully sent to InfluxDB!\n");
    }

    curl_easy_cleanup(curl);
  }
  else {
    fprintf(stderr, "Error when calling curl_easy_init()\n");
  }

  curl_global_cleanup();

  return 0;
}
