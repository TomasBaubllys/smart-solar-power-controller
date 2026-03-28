#include "relay.h"

Relay relays[RELAY_COUNT]{};

void setup_relays(fs::FS& fs, const char* relay_config_file) {
  read_relay_file(fs, relay_config_file);
  for(uint8_t i = 0; i < RELAY_COUNT; ++i) {
    pinMode(RELAY_PINS[i], OUTPUT);
    relays[i].pin = RELAY_PINS[i];
  }
}

void toggle_relay_state_manual(uint8_t idx) {
  if(idx >= RELAY_COUNT) {
    return;
  }

  if(relays[idx].manual_mode == 1) {
      relays[idx].state = relays[idx].state == 1? 0 : 1;
      digitalWrite(relays[idx].pin, relays[idx].state);
  }
}

void read_relay_file(fs::FS& fs, const char* path) {
  File file = fs.open(path);
  if(!file || file.isDirectory()) {
    Serial.printf("Failed to read file %s\n", path);
  }

  char buff[64];
  uint8_t cnt = 0; 
  while(file.available() && cnt < RELAY_COUNT) {
    file.readBytesUntil('\n', buff, sizeof(buff));
    uint8_t phase, state, m_mode;
    sscanf(buff, "%f,%f,%hhu,%hhu,%hhu", &relays[cnt].off_threshold, &relays[cnt].on_threshold, &phase, &state, &m_mode);
    relays[cnt].phase = (phase > PHASE_COUNT - 1? PHASE_COUNT - 1 : phase);
    relays[cnt].state = (state > 0? 1 : 0);
    relays[cnt].manual_mode = (m_mode > 0? 1 : 0);
    ++cnt;
  }

  file.close();
}

void save_relay_info(fs::FS& fs, const char* path) {
  File file = fs.open(path, "w");
  if(!file || file.isDirectory()) {
    Serial.printf("Failed to write to file %s\n", path);
  }
  for(const Relay& rel : relays) {
    file.print(rel.off_threshold);
    file.print(",");
    file.print(rel.on_threshold);
    file.print(",");
    file.print((uint8_t)rel.phase);
    file.print(",");
    file.print((uint8_t)rel.state);
    file.print(",");
    file.println((uint8_t)rel.manual_mode);
  }

  file.close();
  Serial.printf("Succesfully saved relay config to file %s\n", path);
}

void update_relays(AsyncWebSocket& ws) {
  for(Relay& relay : relays) {
    if(!relay.manual_mode) {
      if(relay.on_threshold <= phases[relay.phase].power) {
        relay.state = 1;
        digitalWrite(relay.pin, HIGH);
        send_relay_info_state(ws);
      }
      else if(relay.off_threshold >= phases[relay.phase].power) {
        relay.state = 0;
        digitalWrite(relay.pin, LOW);
        send_relay_info_state(ws);
      }
    }
  }
}

void send_relay_full_info(AsyncWebSocket& ws) {
  char json[512];
  uint16_t pos = 0;
  pos += snprintf(json + pos, sizeof(json) - pos, "{");
  for(uint8_t i = 0; i < RELAY_COUNT; ++i) {
    pos += snprintf(json + pos, sizeof(json) - pos, "\"relay%u\":{\"off_threshold\":%f,\"on_threshold\":%f,\"phase\":%u,\"state\":%u,\"manual_mode\":%u}", i, relays[i].off_threshold, relays[i].on_threshold, relays[i].phase, relays[i].state, relays[i].manual_mode);
    if(i != RELAY_COUNT - 1) 
      pos += snprintf(json + pos, sizeof(json) - pos, ",");
  }
  snprintf(json + pos, sizeof(json) - pos, "}");
  ws.textAll(json);
}

void send_relay_info_state(AsyncWebSocket& ws) {
  char json[256];
  uint16_t pos = 0;
  pos += snprintf(json + pos, sizeof(json) - pos, "{");
  for(uint8_t i = 0; i < RELAY_COUNT; ++i) {
    pos += snprintf(json + pos, sizeof(json) - pos, "\"relay%u\":{\"state\":%u}", i, relays[i].state);
    if(i != RELAY_COUNT - 1) 
      pos += snprintf(json + pos, sizeof(json) - pos, ",");
  }
  snprintf(json + pos, sizeof(json) - pos, "}");
  ws.textAll(json);
}