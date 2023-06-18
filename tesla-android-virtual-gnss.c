#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ws.h>
#include <cJSON.h>
#include <cutils/properties.h>

void webSocketOnConnectionOpened(ws_cli_conn_t *client) {
  char *cli;
  cli = ws_getaddress(client);
  printf("Connection opened, addr: %s\n", cli);
}

void webSocketOnConnectionClosed(ws_cli_conn_t *client) {
  char *cli;
  cli = ws_getaddress(client);
  printf("Connection closed, addr: %s\n", cli);
}

void webSocketOnMessage(__attribute__ ((unused)) ws_cli_conn_t *client,
       const unsigned char *msg, __attribute__ ((unused)) uint64_t size, __attribute__ ((unused)) int type) {
  cJSON *root = cJSON_Parse((char *)msg);
  if (root == NULL) {
    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
  } else {
    cJSON *latitude = cJSON_GetObjectItemCaseSensitive(root, "latitude");
    if (cJSON_IsString(latitude) && (latitude->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.latitude", latitude->valuestring);
    }
    cJSON *longitude = cJSON_GetObjectItemCaseSensitive(root, "longitude");
    if (cJSON_IsString(longitude) && (longitude->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.longitude", longitude->valuestring);
    }
    cJSON *speed = cJSON_GetObjectItemCaseSensitive(root, "speed");
    if (cJSON_IsString(speed) && (speed->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.speed", speed->valuestring);
    } else {
      property_set("persist.tesla-android.gps.speed", "not-available");
    }
    cJSON *bearing = cJSON_GetObjectItemCaseSensitive(root, "bearing");
    if (cJSON_IsString(bearing) && (bearing->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.bearing", bearing->valuestring);
    } else {
      property_set("persist.tesla-android.gps.bearing", "not-available");
    }
    cJSON *vertical_accuracy = cJSON_GetObjectItemCaseSensitive(root, "vertical_accuracy");
    if (cJSON_IsString(vertical_accuracy) && (vertical_accuracy->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.vertical_accuracy", vertical_accuracy->valuestring);
    }
    cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");
    if (cJSON_IsString(timestamp) && (timestamp->valuestring != NULL)) {
      property_set("persist.tesla-android.gps.timestamp", timestamp->valuestring);
    }
    cJSON_Delete(root);
  }
}

void *pingThread(__attribute__ ((unused)) void *arg) {
  while(1) {
    sleep(1);
    ws_ping(NULL, 5);
  }
  return NULL;
}

int main(void)
{
  pthread_t ping_thread;
  pthread_create(&ping_thread, NULL, pingThread, NULL);

  struct ws_events evs;
  evs.onopen    = &webSocketOnConnectionOpened;
  evs.onclose   = &webSocketOnConnectionClosed;
  evs.onmessage = &webSocketOnMessage;
  ws_socket(&evs, 9998, 0, 1000);

  pthread_join(ping_thread, NULL);

  return (0);
}
