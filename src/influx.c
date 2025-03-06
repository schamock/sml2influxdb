// sudo apt install libcurl4-openssl-dev

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>


#define INFLUXDB_URL  "https://influx.ws3:8086/api/v2/write?org=" INFLUX_ORG "&bucket=" INFLUX_BUCKET "&precision=s"

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

int test(void) {
  CURL *curl;
  CURLcode res;

  // Daten im Line Protocol-Format
  const char *data = LINE_PROTOCOL;

  // libcurl initialisieren
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    // Setze die URL (InfluxDB Endpunkt zum Schreiben)
    curl_easy_setopt(curl, CURLOPT_URL, INFLUXDB_URL);

    // Setze HTTP-POST-Daten
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    // Sende die Anfrage
    res = curl_easy_perform(curl);

    // Fehlerbehandlung
    if (res != CURLE_OK) {
      fprintf(stderr, "Fehler beim Senden der Anfrage: %s\n", curl_easy_strerror(res));
    } else {
      printf("Daten erfolgreich in InfluxDB geschrieben!\n");
    }

    // Libcurl-Handle aufräumen
    curl_easy_cleanup(curl);
  }

  // libcurl global aufräumen
  curl_global_cleanup();

  return 0;
}
