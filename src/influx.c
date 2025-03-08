#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h> // boot datatype
#include <stdlib.h>  // exit
#include <time.h>
#include <curl/curl.h> // sudo apt install libcurl4-openssl-dev
#include "config.h"

#define INFLUXDB_URL  "https://influx.ws3:8086/api/v2/write?org=" INFLUX_ORG "&bucket=" INFLUX_BUCKET "&precision=s"
#define LINE_PROTOCOL INFLUX_MEASUREMENT ",location=office value=23.5"

uint16_t formatSmlForInflux(char* influxString, uint16_t maxStringLength, double value180, double value280, double voltageL1,
                        double voltageL2, double voltageL3, double sumActiveInstantaneousPowerTotal,
                        double sumActiveInstantaneousPowerL1, double sumActiveInstantaneousPowerL2,
                        double sumActiveInstantaneousPowerL3 ) {
  uint16_t numChar = snprintf(influxString, maxStringLength, INFLUX_MEASUREMENT " smlValue180=%.4f,smlValue280=%.4f,"
                                "sumActiveInstantaneousPowerTotal=%.2f,sumActiveInstantaneousPowerL1=%.2f," 
                                "sumActiveInstantaneousPowerL2=%.2f,sumActiveInstantaneousPowerL3=%.2f,"
                                "smlVoltageL1=%.1f,smlVoltageL2=%.1f,smlVoltageL3=%.1f %ld\n",
                              value180, value280, sumActiveInstantaneousPowerTotal, sumActiveInstantaneousPowerL1,
                              sumActiveInstantaneousPowerL2, sumActiveInstantaneousPowerL3, voltageL1, voltageL2, voltageL3,
                              time(NULL));
  
  if (numChar >= maxStringLength) {
    fprintf(stderr, "Error: resulting InfluxDB String too long!! Exiting!\n");
    fprintf(stderr, "Max: %d\n", maxStringLength);
    fprintf(stderr, "String: \n %s\n", influxString);
    exit(EXIT_FAILURE);
  }
  return numChar;

}


bool sendInfluxData(char* influxString) {
  CURL *curl;
  CURLcode curlResult;
  struct curl_slist *headerList = {0};
  bool success = false;

  curl_global_init(CURL_GLOBAL_SSL);
  curl = curl_easy_init();

  if (curl) {
    headerList = curl_slist_append(headerList, "Authorization: Token " INFLUX_TOKEN);
    headerList = curl_slist_append(headerList, "Content-Type: text/plain; charset=utf-8");
    headerList = curl_slist_append(headerList, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, INFLUXDB_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, influxString);
    curlResult = curl_easy_perform(curl);

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);

    if (curlResult != CURLE_OK) {
      fprintf(stderr, "Error sending to InfluxDB: %s\n", curl_easy_strerror(curlResult));
    }
    else {
      success = true;
    }
  }
  else {
    fprintf(stderr, "Error when calling curl_easy_init()\n");
  }

  curl_global_cleanup();

  return success;
}
