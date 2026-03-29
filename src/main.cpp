/*
Author: Tomas Baublys
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include "relay.h"
#include "phase.h"
#include "wifi_credentials.h"
#include "modbus.h"

constexpr uint16_t WIFI_RETRY_DELAY = 1000;
constexpr uint16_t TIMER_PRESCALER = 80;
constexpr uint32_t TIMER_DATA_DELAY = 1000000;

hw_timer_t *data_timer = NULL;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

volatile bool request_data = true;

IPAddress local_ip(192, 168, 88, 132);
IPAddress subnet(255, 255, 255, 0); 
IPAddress gateway(192, 168, 88, 1);

void IRAM_ATTR timer_update_data() {
	request_data = true;
}

void handle_ws_data(void* arg, uint8_t* data, size_t len) {
  char buff[128];
  if(len > sizeof(buff) - 1) {
    return;
  }

  memcpy(buff, data, len);
  buff[len] = 0;

  Serial.println(buff);

  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if(info->opcode == WS_TEXT) {
    if(strstr(buff, "{\"type\":\"manual_mode\"") != NULL) {
      uint8_t index = 0;
      uint8_t val = 0;
      sscanf(buff, "{\"type\":\"manual_mode\",\"index\":%hhu,\"value\":%hhu}", &index, &val);
      if(index < RELAY_COUNT) {
        relays[index].manual_mode = val;
      }
    }
    else if(strstr(buff, "{\"type\":\"relay_button\"") != NULL) {
      uint8_t index;
      sscanf(buff, "{\"type\":\"relay_button\",\"index\":%hhu}", &index);
      toggle_relay_state_manual(index);
      send_relay_info_state(ws);
    }
    else if(strstr(buff, "{\"type\":\"set_relay_values\"") != NULL) {
      uint8_t index;
      float on_threshold, off_threshold;
      uint8_t phase;
      int8_t scanned = sscanf(buff, "{\"type\":\"set_relay_values\",\"index\":%hhu,\"on\":%f,\"off\":%f,\"phase\":%hhu}", &index, &on_threshold, &off_threshold, &phase);
      if(scanned != 4){
        Serial.printf("Set failed, only %d values received\n", scanned);
      };       
      if(index >= RELAY_COUNT || phase >= PHASE_COUNT) {
        return;
      }
      relays[index].on_threshold = on_threshold;
      relays[index].off_threshold = off_threshold;
      relays[index].phase = phase;
      save_relay_info(LittleFS, DEFAULT_RELAY_CONFIG_FILE);
      send_relay_full_info(ws);
    }
  }
}

void on_ws_event(AsyncWebSocket* server, AsyncWebSocketClient *client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch(type) {
    case WS_EVT_CONNECT:
      Serial.printf("Websocket client #%u connected from %s\n", client->id(), client->remoteIP());
      send_phase_info(ws);
      send_relay_full_info(ws);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Websocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handle_ws_data(arg, data, len);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);

  if(!LittleFS.begin(false, "/littlefs", 10, "spiffs")){
    Serial.println("Failed to mount LittleFS!");
  }

  if(!WiFi.config(local_ip, gateway, subnet)) {
    Serial.println("Failed to initialize static ip!");
  }

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(WIFI_RETRY_DELAY);
    Serial.print(".");
  }

  Serial.print("Connected, IP:");
  Serial.println(WiFi.localIP());

  data_timer = timerBegin(0, TIMER_PRESCALER, true);
  timerAttachInterrupt(data_timer, &timer_update_data, true);
  timerAlarmWrite(data_timer, TIMER_DATA_DELAY, true);
  timerAlarmEnable(data_timer);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false);
  });

  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/styles.css", "text/css");
  });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  setup_relays(LittleFS, DEFAULT_RELAY_CONFIG_FILE);

  server.addHandler(&ws);
  server.begin();
  ws.onEvent(on_ws_event);
}

void loop() {
  if(request_data) {
    read_voltages_n_currents();
    update_relays(ws);
    send_phase_info(ws);
	  request_data = false;
  }

  ws.cleanupClients();
}
