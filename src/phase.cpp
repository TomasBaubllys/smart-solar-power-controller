#include "phase.h"
#include "modbus.h"

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
  uint8_t slave = DEFAULT_SOLIS_SLAVE_ADDR;
  uint8_t func_code = INV_OP_INF_FUNC_CODE;
  uint32_t data = (uint32_t)PHASE_A_CURR_REG << 16 | 0x0003;
  send_modbus_req(slave, func_code, data);

  return test_cntr++;
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