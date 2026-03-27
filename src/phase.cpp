#include "phase.h"

Phase phases[PHASE_COUNT]{};

// Delete later
float test_cntr = 0;

float read_phase_pwr(const PHASE_NUM phase_num) {
  // NOT IMPLEMENTED YET DUMMY FUNCTION
  test_cntr += 0.3;
  return test_cntr;
}

float read_phase_voltage(const PHASE_NUM phase_num) {
  // NOT IMPLEMENTED YET DUMMY FUNCTION
  test_cntr += 0.3;
  return test_cntr;
}

float read_phase_current(const PHASE_NUM phase_num) {
  // NOT IMPLEMENTED YET DUMMY FUNCTION
  test_cntr += 0.3;
  return test_cntr;
}

void send_phase_info(AsyncWebSocket& ws){
  char json[512];
  uint16_t pos = 0;
  pos += snprintf(json + pos, sizeof(json) - pos, "{");

  for(uint8_t i = 0; i < PHASE_COUNT; ++i) {
    pos += snprintf(json + pos, sizeof(json) - pos, "\"phase%u\":{\"voltage\":%f,\"current\":%f,\"power\":%f}", i, phases[i].voltage, phases[i].current, phases[i].power);
    if(i != PHASE_COUNT - 1) 
      pos += snprintf(json + pos, sizeof(json) - pos, ",");
  }

  snprintf(json + pos, sizeof(json) - pos, "}");
  ws.textAll(json);
}