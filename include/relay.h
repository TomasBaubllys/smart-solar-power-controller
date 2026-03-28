#include <sys/_stdint.h>
#ifndef RELAY_H_INCLUDED
#define RELAY_H_INCLUDED

#include <stdint.h>
#include <LittleFS.h>
#include "phase.h"

constexpr uint8_t RELAY_COUNT = 4;
constexpr const char* DEFAULT_RELAY_CONFIG_FILE = "/relay_config.txt";

constexpr uint8_t RELAY_PINS[] = {12, 13, 14, 15};

struct Relay{
  float off_threshold;
  float on_threshold;
  uint8_t pin;
  uint8_t phase:4;
  uint8_t state:1;
  uint8_t manual_mode:1;
};

extern Relay relays[RELAY_COUNT];

// only works if the relay is in manual mode, otherwise does nothing
void toggle_relay_state_manual(uint8_t idx);

void setup_relays(fs::FS& fs, const char* relay_config_file);

void save_relay_info(fs::FS& fs, const char* path);

void read_relay_file(fs::FS& fs, const char* path); 

void update_relays(AsyncWebSocket& ws);

void send_relay_full_info(AsyncWebSocket& ws);

void send_relay_info_state(AsyncWebSocket& ws);

#endif // RELAY_H_INCLUDED